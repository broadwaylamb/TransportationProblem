//
//  InputDelegate.cpp
//  app
//
//  Created by Sergej Jaskiewicz on 18/12/2017.
//

#include <QLineEdit>
#include <QIntValidator>
#include "InputItemDelegate.h"

InputItemDelegate::InputItemDelegate(QObject *parent):
    OutputItemDelegate(parent) {}

QWidget *InputItemDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem&,
                                         const QModelIndex &index) const {
  
  if (index.column() == index.model()->columnCount() - 1 &&
      index.row()    == index.model()->rowCount() - 1) {
    return nullptr;
  }
  
  QLineEdit *editor = new QLineEdit(parent);
  
  auto validator = new QIntValidator;
  validator->setRange(1, std::numeric_limits<int>::max());
  
  editor->setValidator(validator);
  return editor;
}

void InputItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QLineEdit *line = static_cast<QLineEdit*>(editor);
  line->setText(value);
}


void InputItemDelegate::setModelData(QWidget *editor,
                                     QAbstractItemModel *model,
                                     const QModelIndex &index) const {
  QLineEdit *line = static_cast<QLineEdit*>(editor);
  int value = line->text().toInt();
    
  if (shouldSetDataCallback == nullptr || shouldSetDataCallback(index, value)) {
    model->setData(index, value);
  }
}
