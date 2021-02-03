/*
  SPDX-FileCopyrightText: 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "categoryselectdialog.h"
#include "categoryhierarchyreader.h"
#include "ui_categoryselectdialog_base.h"

#include <CalendarSupport/CategoryConfig>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

class CategorySelectWidgetBase : public QWidget, public Ui::CategorySelectDialog_base
{
public:
    CategorySelectWidgetBase(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);

        mButtonClear->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-rtl")));
        mButtonEdit->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    }
};

CategorySelectWidget::CategorySelectWidget(CategoryConfig *cc, QWidget *parent)
    : QWidget(parent)
    , mCategoryConfig(cc)
{
    auto topL = new QHBoxLayout(this);
    topL->setContentsMargins(0, 0, 0, 0);
    mWidgets = new CategorySelectWidgetBase(this);
    topL->addWidget(mWidgets);
    connect(mWidgets->mButtonEdit, &QAbstractButton::clicked, this, &CategorySelectWidget::editCategories);
    connect(mWidgets->mButtonClear, &QAbstractButton::clicked, this, &CategorySelectWidget::clear);
}

CategorySelectWidget::~CategorySelectWidget()
{
}

AutoCheckTreeWidget *CategorySelectWidget::listView() const
{
    return mWidgets->mCategories;
}

void CategorySelectWidget::hideButton()
{
    mWidgets->mButtonEdit->hide();
    mWidgets->mButtonClear->hide();
}

void CategorySelectWidget::setCategories(const QStringList &categoryList)
{
    mWidgets->mCategories->clear();
    mCategoryList.clear();

    QStringList cats = mCategoryConfig->customCategories();
    for (QStringList::ConstIterator it = categoryList.begin(), end = categoryList.end(); it != end; ++it) {
        if (!cats.contains(*it)) {
            cats.append(*it);
        }
    }
    mCategoryConfig->setCustomCategories(cats);
    CategoryHierarchyReaderQTreeWidget(mWidgets->mCategories).read(cats);
}

void CategorySelectWidget::setSelected(const QStringList &selList)
{
    clear();
    bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);
    for (QStringList::ConstIterator it = selList.begin(), end = selList.end(); it != end; ++it) {
        QStringList path = CategoryHierarchyReader::path(*it);
        QTreeWidgetItem *item = mWidgets->mCategories->itemByPath(path);
        if (item) {
            item->setCheckState(0, Qt::Checked);
        }
    }
    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

static QStringList getSelectedCategoriesFromCategoriesView(AutoCheckTreeWidget *categoriesView)
{
    QStringList categories;

    QTreeWidgetItemIterator it(categoriesView, QTreeWidgetItemIterator::Checked);
    while (*it) {
        QStringList path = categoriesView->pathByItem(*it++);
        if (!path.isEmpty()) {
            path.replaceInStrings(CategoryConfig::categorySeparator, QLatin1Char('\\') + CategoryConfig::categorySeparator);
            categories.append(path.join(CategoryConfig::categorySeparator));
        }
    }

    return categories;
}

void CategorySelectWidget::clear()
{
    bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);

    QTreeWidgetItemIterator it(mWidgets->mCategories);
    while (*it) {
        (*it++)->setCheckState(0, Qt::Unchecked);
    }

    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

void CategorySelectWidget::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->mCategories->setAutoCheckChildren(autoselectChildren);
}

void CategorySelectWidget::hideHeader()
{
    mWidgets->mCategories->header()->hide();
}

QStringList CategorySelectWidget::selectedCategories(QString &categoriesStr)
{
    mCategoryList = getSelectedCategoriesFromCategoriesView(listView());
    categoriesStr = mCategoryList.join(QLatin1String(", "));
    return mCategoryList;
}

QStringList CategorySelectWidget::selectedCategories() const
{
    return mCategoryList;
}

void CategorySelectWidget::setCategoryList(const QStringList &categories)
{
    mCategoryList = categories;
}

CategorySelectDialog::CategorySelectDialog(CategoryConfig *cc, QWidget *parent)
    : QDialog(parent)
    , d(nullptr)
{
    setWindowTitle(i18nc("@title:window", "Select Categories"));
    auto mainLayout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel /*| QDialogButtonBox::Help*/ | QDialogButtonBox::Apply, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CategorySelectDialog::reject);

    QWidget *page = new QWidget;
    auto lay = new QVBoxLayout(page);
    lay->setContentsMargins(0, 0, 0, 0);

    mWidgets = new CategorySelectWidget(cc, this);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
    mWidgets->setObjectName(QStringLiteral("CategorySelection"));
    mWidgets->hideHeader();
    lay->addWidget(mWidgets);

    mWidgets->setCategories();
    mWidgets->listView()->setFocus();

    connect(mWidgets, &CategorySelectWidget::editCategories, this, &CategorySelectDialog::editCategories);

    connect(okButton, &QPushButton::clicked, this, &CategorySelectDialog::slotOk);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &CategorySelectDialog::slotApply);
}

CategorySelectDialog::~CategorySelectDialog()
{
    delete mWidgets;
}

QStringList CategorySelectDialog::selectedCategories() const
{
    return mWidgets->selectedCategories();
}

void CategorySelectDialog::slotApply()
{
    QString categoriesStr;
    QStringList categories = mWidgets->selectedCategories(categoriesStr);
    Q_EMIT categoriesSelected(categories);
    Q_EMIT categoriesSelected(categoriesStr);
}

void CategorySelectDialog::slotOk()
{
    slotApply();
    accept();
}

void CategorySelectDialog::updateCategoryConfig()
{
    QString tmp;
    QStringList selected = mWidgets->selectedCategories(tmp);
    mWidgets->setCategories();
    mWidgets->setSelected(selected);
}

void CategorySelectDialog::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->setAutoselectChildren(autoselectChildren);
}

void CategorySelectDialog::setCategoryList(const QStringList &categories)
{
    mWidgets->setCategoryList(categories);
}

void CategorySelectDialog::setSelected(const QStringList &selList)
{
    mWidgets->setSelected(selList);
}
