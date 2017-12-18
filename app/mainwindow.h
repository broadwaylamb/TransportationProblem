#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "TransportationProblem.h"

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();
    
public slots:
  void updateSupplyCount(int newCount);
  void updateDemandCount(int newCount);
  void clearState();
  void generateRandomInput();
  void solve();
  void loadProblem();
  void saveSolution();
  
private:
  Ui::MainWindow* ui;
  QStandardItemModel* inputModel;
  QStandardItemModel* outputModel;
  TProblem::TransportationProblem* problem = nullptr;
  std::vector<TProblem::Quantity> supply;
  std::vector<TProblem::Quantity> demand;
  
  bool pivotFound = false;
  
  void clearInput();
  void clearOutput();

  void recreateInputTable();
  void resizeTables();
  
  void parseInput();
  void outputProblem();
  
  bool writeSolutionToFile(QString fileName);
  bool readProblemFromFile(QString fileName);
  
  static const QString solveButtonInitialText;
  static const QString solveButtonFindOptimumText;
};

template<typename Result>
Result iterate(QStandardItemModel* model,
                const std::function<Result(QModelIndex)> &lambda) {
  for (int row = 0; row < model->rowCount(); ++row) {
    for (int column = 0; column < model->columnCount(); ++column) {
      auto index = model->index(row, column);
      lambda(index);
    }
  }
}

#endif // MAINWINDOW_H
