//
//  Model.cpp
//  app
//
//  Created by Sergej Jaskiewicz on 24/12/2017.
//

#include "Model.h"

Model::Model(QObject *parent): QStandardItemModel(parent) {}

QVariant Model::data(const QModelIndex &index, int role) const {
    if (role == Qt::TextAlignmentRole) { return Qt::AlignCenter; }
    if (index.row() == rowCount() - 1 && index.column() == columnCount() - 1) {
        return QVariant();
    }
    return QStandardItemModel::data(index, role);
}

bool Model::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.row() == rowCount() - 1 && index.column() == columnCount() - 1) {
        return false;
    } else {
        return QStandardItemModel::setData(index, value, role);
    }
}
