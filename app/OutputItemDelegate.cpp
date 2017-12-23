//
//  OutputItemDelegate.cpp
//  app
//
//  Created by Sergej Jaskiewicz on 23/12/2017.
//

#include "OutputItemDelegate.h"
#include <QPainter>

OutputItemDelegate::OutputItemDelegate(QObject *parent):
QItemDelegate(parent) {}

void OutputItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const {
  
  bool isLastRow = index.row() == index.model()->rowCount() - 1;
  bool isLastColumn = index.column() == index.model()->columnCount() - 1;
  
  if (isLastRow != isLastColumn) {
    
    painter->fillRect(option.rect, option.palette.alternateBase());
    
    QPointF labelPosition = option.rect.bottomLeft() + QPointF(5, -5);
    QString labelText = (isLastColumn ? "S%1" : "D%1");
    labelText = labelText
      .arg((isLastColumn ? index.row() : index.column()) + 1);
    painter->setPen(option.palette.mid().color());
    painter->drawText(labelPosition, labelText);
    
  } else if (isLastRow && isLastColumn) {
    painter->fillRect(option.rect, option.palette.mid());
  }
  
  // Set center alignment
  QString text = index.model()->data(index, Qt::DisplayRole).toString();
  auto newOption = option;
  newOption.displayAlignment = Qt::AlignCenter;
  drawDisplay(painter, newOption, newOption.rect, text);
  drawFocus(painter, newOption, newOption.rect);
}
