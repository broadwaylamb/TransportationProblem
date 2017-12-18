//
//  InputDelegate.cpp
//  app
//
//  Created by Sergej Jaskiewicz on 18/12/2017.
//

#include <QLineEdit>
#include <QIntValidator>
#include "InputDelegate.h"

InputDelegate::InputDelegate(QObject *parent): QItemDelegate(parent) {}

QWidget *InputDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem&,
                                     const QModelIndex &index) const {
  
  if (index.column() == index.model()->columnCount() - 1 &&
      index.row()    == index.model()->rowCount() - 1) {
    return nullptr;
  }
  
  QLineEdit *editor = new QLineEdit(parent);
  
  auto validator = new QIntValidator;
  
  editor->setValidator(validator);
  return editor;
}

void InputDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QLineEdit *line = static_cast<QLineEdit*>(editor);
  line->setText(value);
}


void InputDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const {
  QLineEdit *line = static_cast<QLineEdit*>(editor);
  QString value = line->text();
  model->setData(index, value);
}
