/*
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencecompletionpriority.h"
using namespace Qt::Literals::StringLiterals;

#include "ui_dialogdesktop.h"

#include <KCalendarCore/Todo>

using namespace IncidenceEditorNG;

class IncidenceEditorNG::IncidenceCompletionPriorityPrivate
{
    IncidenceCompletionPriority *const q;

public:
    explicit IncidenceCompletionPriorityPrivate(IncidenceCompletionPriority *parent)
        : q(parent)
    {
    }

    Ui::EventOrTodoDesktop *mUi = nullptr;
    int mOrigPercentCompleted = -1;

    void sliderValueChanged(int);
};

void IncidenceCompletionPriorityPrivate::sliderValueChanged(int value)
{
    if (q->sender() == mUi->mCompletionSlider) {
        mOrigPercentCompleted = -1;
    }

    mUi->mCompletedLabel->setText(u"%1%"_s.arg(value));
    q->checkDirtyStatus();
}

IncidenceCompletionPriority::IncidenceCompletionPriority(Ui::EventOrTodoDesktop *ui)
    : d(new IncidenceCompletionPriorityPrivate(this))
{
    Q_ASSERT(ui != nullptr);
    setObjectName("IncidenceCompletionPriority"_L1);

    d->mUi = ui;

    d->sliderValueChanged(d->mUi->mCompletionSlider->value());
    d->mUi->mCompletionPriorityWidget->hide();
    d->mUi->mTaskLabel->hide();
    const QFontMetrics metrics(d->mUi->mCompletedLabel->font());
    d->mUi->mCompletedLabel->setMinimumWidth(metrics.boundingRect(u"100%"_s).width());
    d->mUi->mTaskSeparator->hide();

    connect(d->mUi->mCompletionSlider, &QSlider::valueChanged, this, [this](int val) {
        d->sliderValueChanged(val);
    });
    connect(d->mUi->mPriorityCombo, &QComboBox::currentIndexChanged, this, &IncidenceCompletionPriority::checkDirtyStatus);
}

IncidenceCompletionPriority::~IncidenceCompletionPriority() = default;

void IncidenceCompletionPriority::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;

    // TODO priority might be valid for other incidence types as well
    // only for Todos
    KCalendarCore::Todo::Ptr const todo = IncidenceCompletionPriority::incidence<KCalendarCore::Todo>();
    if (todo == nullptr) {
        mWasDirty = false;
        return;
    }

    d->mUi->mCompletionPriorityWidget->show();
    d->mUi->mTaskLabel->show();
    d->mUi->mTaskSeparator->show();

    d->mOrigPercentCompleted = todo->percentComplete();
    d->mUi->mCompletionSlider->blockSignals(true);
    d->mUi->mCompletionSlider->setValue(todo->percentComplete());
    d->sliderValueChanged(d->mUi->mCompletionSlider->value());
    d->mUi->mCompletionSlider->blockSignals(false);

    d->mUi->mPriorityCombo->blockSignals(true);
    d->mUi->mPriorityCombo->setCurrentIndex(todo->priority());
    d->mUi->mPriorityCombo->blockSignals(false);

    mWasDirty = false;
}

void IncidenceCompletionPriority::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    // TODO priority might be valid for other incidence types as well
    // only for Todos
    KCalendarCore::Todo::Ptr const todo = IncidenceCompletionPriority::incidence<KCalendarCore::Todo>(incidence);
    if (todo == nullptr) {
        return;
    }

    // we only have multiples of ten on our combo. If the combo did not change its value,
    // see if we have an original value to restore
    if (d->mOrigPercentCompleted != -1) {
        todo->setPercentComplete(d->mOrigPercentCompleted);
    } else {
        const int pct = d->mUi->mCompletionSlider->value();
        if (pct >= 100) {
            todo->setCompleted(QDateTime::currentDateTimeUtc());
            todo->setStatus(KCalendarCore::Incidence::StatusCompleted);
        } else {
            todo->setCompleted(false);
            todo->setStatus(pct <= 0 ? KCalendarCore::Incidence::StatusNone : KCalendarCore::Incidence::StatusInProcess);
        }
        todo->setPercentComplete(pct);
    }
    todo->setPriority(d->mUi->mPriorityCombo->currentIndex());
}

bool IncidenceCompletionPriority::isDirty() const
{
    KCalendarCore::Todo::Ptr const todo = IncidenceCompletionPriority::incidence<KCalendarCore::Todo>();

    if (!todo) {
        return false;
    }

    if (d->mUi->mCompletionSlider->value() != todo->percentComplete()) {
        return true;
    }

    if (d->mUi->mPriorityCombo->currentIndex() != todo->priority()) {
        return true;
    }

    return false;
}

#include "moc_incidencecompletionpriority.cpp"
