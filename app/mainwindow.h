#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "Model.h"
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
  void about();
  
protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent* event) override;
  
private:
  Ui::MainWindow* ui;
  Model* inputModel;
  Model* outputModel;
  TProblem::TransportationProblem* problem = nullptr;
  std::vector<TProblem::Quantity> supply;
  std::vector<TProblem::Quantity> demand;
  
  bool pivotFound = false;
  
  bool validateInput();
  
  void clearInput();
  void clearOutput();

  void recreateInputTable();
  void resizeTables();
  
  void parseInput();
  void outputProblem();
  
  bool writeSolutionToFile(QString fileName);
  bool readProblemFromFile(QString fileName);
  
  void alertOpenFileError(QString fileName);
  
  static const char* solveButtonInitialText;
  static const char* solveButtonFindOptimumText;
  static const char* solveButtonOptimumFoundText;
};

void iterate(Model* model, const std::function<void(QModelIndex)> &lambda);

#endif // MAINWINDOW_H
