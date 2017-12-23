//
//  InputItemDelegate.h
//  app
//
//  Created by Sergej Jaskiewicz on 18/12/2017.
//

#ifndef InputItemDelegate_h
#define InputItemDelegate_h

#include "OutputItemDelegate.h"

class InputItemDelegate: public OutputItemDelegate {
  
public:
  explicit InputItemDelegate(QObject *parent = 0);
  
protected:
  QWidget* createEditor(QWidget *parent,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
  
  void setEditorData(QWidget* editor,
                     const QModelIndex & index) const;
  
  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex &index) const;
};

#endif /* InputDelegate_h */
