/*
  SPDX-FileCopyrightText: 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "categorydialog.h"
#include "categoryhierarchyreader.h"
#include "ui_categorydialog_base.h"

#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/CategoryConfig>

#include <QIcon>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

class CategoryWidgetBase : public QWidget, public Ui::CategoryDialog_base
{
public:
    CategoryWidgetBase(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

CategoryWidget::CategoryWidget(CategoryConfig *cc, QWidget *parent)
    : QWidget(parent)
    , mCategoryConfig(cc)
{
    QHBoxLayout *topL = new QHBoxLayout(this);
    topL->setContentsMargins(0, 0, 0, 0);
    mWidgets = new CategoryWidgetBase(this);
    topL->addWidget(mWidgets);

    mWidgets->mButtonAdd->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    mWidgets->mButtonRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    mWidgets->mLineEdit->setPlaceholderText(i18n("Click to add a new category"));

    connect(mWidgets->mLineEdit, &QLineEdit::textChanged,
            this, &CategoryWidget::handleTextChanged);

    mWidgets->mButtonAdd->setEnabled(false);
    mWidgets->mButtonRemove->setEnabled(false);
    mWidgets->mColorCombo->setEnabled(false);

    connect(mWidgets->mCategories, &QTreeWidget::itemSelectionChanged,
            this, &CategoryWidget::handleSelectionChanged);

    connect(mWidgets->mButtonAdd, &QAbstractButton::clicked,
            this, &CategoryWidget::addCategory);

    connect(mWidgets->mButtonRemove, &QAbstractButton::clicked,
            this, &CategoryWidget::removeCategory);

    connect(mWidgets->mColorCombo, &KColorCombo::activated,
            this, &CategoryWidget::handleColorChanged);
}

CategoryWidget::~CategoryWidget()
{
}

AutoCheckTreeWidget *CategoryWidget::listView() const
{
    return mWidgets->mCategories;
}

void CategoryWidget::hideButton()
{
}

void CategoryWidget::setCategories(const QStringList &categoryList)
{
    mWidgets->mCategories->clear();
    mCategoryList.clear();

    QStringList cats = mCategoryConfig->customCategories();
    for (QStringList::ConstIterator it = categoryList.begin(), end = categoryList.end(); it != end;
         ++it) {
        if (!cats.contains(*it)) {
            cats.append(*it);
        }
    }
    mCategoryConfig->setCustomCategories(cats);
    CategoryHierarchyReaderQTreeWidget(mWidgets->mCategories).read(cats);
}

void CategoryWidget::setSelected(const QStringList &selList)
{
    clear();
    const bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
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

static QStringList getSelectedCategories(AutoCheckTreeWidget *categoriesView)
{
    QStringList categories;

    QTreeWidgetItemIterator it(categoriesView, QTreeWidgetItemIterator::Checked);
    while (*it) {
        QStringList path = categoriesView->pathByItem(*it++);
        if (!path.isEmpty()) {
            path.replaceInStrings(CategoryConfig::categorySeparator, QLatin1Char('\\')
                                  +CategoryConfig::categorySeparator);
            categories.append(path.join(CategoryConfig::categorySeparator));
        }
    }

    return categories;
}

void CategoryWidget::clear()
{
    const bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);

    QTreeWidgetItemIterator it(mWidgets->mCategories);
    while (*it) {
        (*it++)->setCheckState(0, Qt::Unchecked);
    }

    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

void CategoryWidget::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->mCategories->setAutoCheckChildren(autoselectChildren);
}

void CategoryWidget::hideHeader()
{
    mWidgets->mCategories->header()->hide();
}

QStringList CategoryWidget::selectedCategories(QString &categoriesStr)
{
    mCategoryList = getSelectedCategories(listView());
    categoriesStr = mCategoryList.join(QLatin1String(", "));
    return mCategoryList;
}

QStringList CategoryWidget::selectedCategories() const
{
    return mCategoryList;
}

void CategoryWidget::setCategoryList(const QStringList &categories)
{
    mCategoryList = categories;
}

void CategoryWidget::addCategory()
{
    QTreeWidgetItem *newItem = new QTreeWidgetItem(listView(),
                                                   QStringList(mWidgets->mLineEdit->text()));
    listView()->scrollToItem(newItem);
    listView()->clearSelection();
    newItem->setSelected(true);
}

void CategoryWidget::removeCategory()
{
    // Multi-select not supported, only one selected
    QTreeWidgetItem *itemToDelete = listView()->selectedItems().first();
    delete itemToDelete;
}

void CategoryWidget::handleTextChanged(const QString &newText)
{
    mWidgets->mButtonAdd->setEnabled(!newText.isEmpty());
}

void CategoryWidget::handleSelectionChanged()
{
    const bool hasSelection = !listView()->selectedItems().isEmpty();
    mWidgets->mButtonRemove->setEnabled(hasSelection);
    mWidgets->mColorCombo->setEnabled(hasSelection);

    if (hasSelection) {
        const QTreeWidgetItem *item = listView()->selectedItems().first();
        const QColor &color = KCalPrefs::instance()->categoryColor(item->text(0));
        if (color.isValid()) {
            mWidgets->mColorCombo->setColor(color);
            // update is needed. bug in KColorCombo?
            mWidgets->mColorCombo->update();
        }
    }
}

void CategoryWidget::handleColorChanged(const QColor &newColor)
{
    if (!listView()->selectedItems().isEmpty()) {
        const QTreeWidgetItem *item = listView()->selectedItems().first();
        const QString category = item->text(0);
        if (newColor.isValid()) {
            KCalPrefs::instance()->setCategoryColor(category, newColor);
        }
    }
}

CategoryDialog::CategoryDialog(CategoryConfig *cc, QWidget *parent)
    : QDialog(parent)
    , d(nullptr)
{
    setWindowTitle(i18nc("@title:window", "Select Categories"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel /*| QDialogButtonBox::Help*/ | QDialogButtonBox::Apply,
        this);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CategoryDialog::reject);

    QWidget *page = new QWidget;
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(0, 0, 0, 0);

    mWidgets = new CategoryWidget(cc, this);
    mCategoryConfig = cc;
    mWidgets->setObjectName(QStringLiteral("CategorySelection"));
    mWidgets->hideHeader();
    lay->addWidget(mWidgets);

    mWidgets->setCategories();
    mWidgets->listView()->setFocus();

    connect(okButton, &QPushButton::clicked, this, &CategoryDialog::slotOk);
    connect(buttonBox->button(
                QDialogButtonBox::Apply), &QPushButton::clicked, this, &CategoryDialog::slotApply);
}

CategoryDialog::~CategoryDialog()
{
    delete mWidgets;
}

QStringList CategoryDialog::selectedCategories() const
{
    return mWidgets->selectedCategories();
}

void CategoryDialog::slotApply()
{
    QStringList l;

    QStringList path;
    QTreeWidgetItemIterator it(mWidgets->listView());
    while (*it) {
        path = mWidgets->listView()->pathByItem(*it++);
        path.replaceInStrings(
            CategoryConfig::categorySeparator,
            QLatin1Char('\\') + CategoryConfig::categorySeparator);
        l.append(path.join(CategoryConfig::categorySeparator));
    }
    mCategoryConfig->setCustomCategories(l);
    mCategoryConfig->writeConfig();

    QString categoriesStr;
    QStringList categories = mWidgets->selectedCategories(categoriesStr);
    Q_EMIT categoriesSelected(categories);
    Q_EMIT categoriesSelected(categoriesStr);
}

void CategoryDialog::slotOk()
{
    slotApply();
    accept();
}

void CategoryDialog::updateCategoryConfig()
{
    QString tmp;
    QStringList selected = mWidgets->selectedCategories(tmp);
    mWidgets->setCategories();
    mWidgets->setSelected(selected);
}

void CategoryDialog::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->setAutoselectChildren(autoselectChildren);
}

void CategoryDialog::setCategoryList(const QStringList &categories)
{
    mWidgets->setCategoryList(categories);
}

void CategoryDialog::setSelected(const QStringList &selList)
{
    mWidgets->setSelected(selList);
}
