//
//  Model.h
//  app
//
//  Created by Sergej Jaskiewicz on 24/12/2017.
//

#ifndef Model_h
#define Model_h

#include <QStandardItemModel>

class Model: public QStandardItemModel {
public:
  explicit Model(QObject *parent = nullptr);
  
  QVariant data (const QModelIndex &index, int role) const;

  bool setData(const QModelIndex &index,
               const QVariant &value,
               int role = Qt::EditRole);
};


#endif /* Model_h */
