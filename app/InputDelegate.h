//
//  InputDelegate.h
//  app
//
//  Created by Sergej Jaskiewicz on 18/12/2017.
//

#ifndef InputDelegate_hpp
#define InputDelegate_hpp

#include <QItemDelegate>

class InputDelegate: public QItemDelegate {
    Q_OBJECT
public:
    explicit InputDelegate(QObject *parent = 0);
    
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
