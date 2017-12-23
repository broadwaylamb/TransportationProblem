//
//  OutputItemDelegate.h
//  app
//
//  Created by Sergej Jaskiewicz on 23/12/2017.
//

#ifndef OutputItemDelegate_h
#define OutputItemDelegate_h

#include <QItemDelegate>

class OutputItemDelegate: public QItemDelegate {

    
public:
    explicit OutputItemDelegate(QObject *parent = 0);
    
protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

#endif /* OutputItemDelegate_h */
