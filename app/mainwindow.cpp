#include <QFileDialog>
#include <QRandomGenerator>
#include <QTextStream>
#include <QMessageBox>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include "Model.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "InputItemDelegate.h"

#define SUPPLY_MIN_COUNT 2
#define SUPPLY_MAX_COUNT 50
#define DEMAND_MIN_COUNT SUPPLY_MIN_COUNT
#define DEMAND_MAX_COUNT SUPPLY_MAX_COUNT

const char* MainWindow::solveButtonInitialText =
  QT_TR_NOOP("Apply North-West corner method");

const char* MainWindow::solveButtonFindOptimumText =
  QT_TR_NOOP("Find optimal solution using the stepping stone method");

const char* MainWindow::solveButtonOptimumFoundText =
  QT_TR_NOOP("The optimal solution is found");

MainWindow::MainWindow(QWidget *parent):
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  inputModel(new Model(this)),
  outputModel(new Model(this)) {

  ui->setupUi(this);
  
  recreateInputTable();
    
  ui->input_table->setItemDelegate(new InputItemDelegate(this));
  ui->output_table->setItemDelegate(new OutputItemDelegate(this));
  
  ui->input_table->
    horizontalHeader()->
    setSectionResizeMode(QHeaderView::Stretch);
  ui->input_table->
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  ui->output_table->
    horizontalHeader()->
  setSectionResizeMode(QHeaderView::Stretch);
  ui->output_table->
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  ui->input_table->setModel(inputModel);
  ui->input_table->verticalHeader()->hide();
  ui->input_table->horizontalHeader()->hide();
  
  ui->output_table->setModel(outputModel);
  ui->output_table->verticalHeader()->hide();
  ui->output_table->horizontalHeader()->hide();
  
  connect(ui->supply_spin,                  SIGNAL(valueChanged(int)),
          this,                             SLOT(updateSupplyCount(int)));
  
  connect(ui->demand_spin,                  SIGNAL(valueChanged(int)),
          this,                             SLOT(updateDemandCount(int)));
  
  connect(ui->clear_button,                 SIGNAL(clicked(void)),
          this,                             SLOT(clearState(void)));
  
  connect(ui->generate_random_input_button, SIGNAL(clicked(void)),
          this,                             SLOT(generateRandomInput(void)));
  
  connect(ui->solve_button,                 SIGNAL(clicked(void)),
          this,                             SLOT(solve(void)));
  
  connect(ui->action_LoadProblem,           SIGNAL(triggered(void)),
          this,                             SLOT(loadProblem(void)));
  
  connect(ui->action_SaveSolution,          SIGNAL(triggered(void)),
          this,                             SLOT(saveSolution(void)));
  
  connect(ui->action_About,                 SIGNAL(triggered(void)),
          this,                             SLOT(about(void)));  
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::updateSupplyCount(int newCount) {
  if (newCount < SUPPLY_MIN_COUNT && newCount > SUPPLY_MAX_COUNT) { return; }
  inputModel->setRowCount(newCount + 1);
  resizeTables();
  validateInput();
  clearOutput();
}

void MainWindow::updateDemandCount(int newCount) {
  if (newCount < DEMAND_MIN_COUNT && newCount > DEMAND_MAX_COUNT) { return; }
  inputModel->setColumnCount(newCount + 1);
  resizeTables();
  validateInput();
  clearOutput();
}

void MainWindow::clearState() {
  clearInput();
  clearOutput();
}

bool MainWindow::validateInput() {
  
  bool isValid = true;
  bool isEmpty = true;
  
  iterate(inputModel, [&](QModelIndex index) {
    if (index.row() == index.model()->rowCount() - 1 &&
        index.column() == index.model()->columnCount() - 1) {
      return;
    }
    isEmpty = false;
    isValid = isValid && index.data().isValid();
  });
  
  isValid = isValid && !isEmpty;
  
  ui->solve_button->setEnabled(isValid);
  
  return isValid;
}

void MainWindow::clearInput() {
  inputModel->clear();
  ui->solve_button->setEnabled(false);
  recreateInputTable();
}

void MainWindow::clearOutput() {
  
  if (problem != nullptr) {
    delete problem;
    problem = nullptr;
  }
  
  pivotFound = false;
  outputModel->clear();
  ui->solve_button->setText(tr(MainWindow::solveButtonInitialText));
  ui->total_cost_label->setNum(0);
  ui->degenerate_case_label->hide();
}

void MainWindow::generateRandomInput() {
  
  clearOutput();
  
  iterate(inputModel, [this](QModelIndex index) {
      
    if (index.column() == inputModel->columnCount() - 1 &&
        index.row() == inputModel->rowCount() - 1) {
      return;
    }
    
    // Generate supply
    if (index.column() == inputModel->columnCount() - 1) {
      inputModel->setData(index, QRandomGenerator::global()->bounded(1, 70));
      return;
    }
    
    // Generate demand
    if (index.row() == inputModel->rowCount() - 1) {
      inputModel->setData(index, QRandomGenerator::global()->bounded(1, 70));
      return;
    }
    
    // Generate cost matrix
    inputModel->setData(index, QRandomGenerator::global()->bounded(1, 10));
  });
  
  ui->solve_button->setEnabled(true);
}

void MainWindow::solve() {
  
  if (!pivotFound || problem == nullptr) {
    parseInput();
    
    problem->fixImbalance();
    
    this->supply = problem->supply;
    this->demand = problem->demand;
    
    problem->northWestCorner();
    
    outputProblem();
    
    pivotFound = true;
    
    ui->solve_button->setText(tr(MainWindow::solveButtonFindOptimumText));
  } else if (problem != nullptr) {
    problem->steppingStone();
    
    outputProblem();
    ui->solve_button->setText(tr(MainWindow::solveButtonOptimumFoundText));
    ui->solve_button->setEnabled(false);
  }
  
  ui->total_cost_label->setNum(problem->totalCost());
  ui->degenerate_case_label->setHidden(!problem->isDegenerate());
}

void MainWindow::parseInput() {
  
  std::vector<TProblem::Quantity> supply(inputModel->rowCount() - 1, 0);
  std::vector<TProblem::Quantity> demand(inputModel->columnCount() - 1, 0);
  TProblem::TransportationProblem::CostMatrix costMatrix(
    supply.size(),
    std::vector<TProblem::Currency>(demand.size(), 0)
  );
  
  iterate(inputModel,
                [this, &supply, &demand, &costMatrix](QModelIndex index) {
                  
    if (index.column() == inputModel->columnCount() - 1 &&
        index.row() == inputModel->rowCount() - 1) {
      return;
    }
    
    // Parse supply
    if (index.column() == inputModel->columnCount() - 1) {
      supply[index.row()] = index.data().toInt();
      return;
    }
    
    // Parse demand
    if (index.row() == inputModel->rowCount() - 1) {
      demand[index.column()] = index.data().toInt();
      return;
    }
    
    // Parse cost matrix
    costMatrix[index.row()][index.column()] = index.data().toInt();
  });
  
  problem = new TProblem::TransportationProblem(supply, demand, costMatrix);
}

void MainWindow::outputProblem() {
  
  outputModel->clear();
  
  outputModel->setRowCount(supply.size() + 1);
  outputModel->setColumnCount(demand.size() + 1);
  
  iterate(outputModel, [this](QModelIndex index) {
    
    if (index.column() == outputModel->columnCount() - 1 &&
        index.row() == outputModel->rowCount() - 1) {
      return;
    }
    
    // Output supply
    if (index.column() == outputModel->columnCount() - 1) {
      outputModel->setData(index, supply[index.row()]);
      return;
    }
    
    // Output demand
    if (index.row() == outputModel->rowCount() - 1) {
      outputModel->setData(index, demand[index.column()]);
      return;
    }
    
    // Output shipments
    if (auto shipment = problem->shipments[index.row()][index.column()]) {
      auto quantity = shipment->quantity;
      outputModel->setData(index, quantity);
    }
  });
}

void MainWindow::recreateInputTable() {
  updateSupplyCount(ui->supply_spin->value());
  updateDemandCount(ui->demand_spin->value());
}

void MainWindow::resizeTables() {
  ui->input_table->verticalHeader()->resizeSections(QHeaderView::Stretch);
  ui->input_table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
  ui->output_table->verticalHeader()->resizeSections(QHeaderView::Stretch);
  ui->output_table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
}

static int calculateFieldWidth(Model* model) {
  int fieldWidth = 0;
  iterate(model, [&fieldWidth](QModelIndex index) {
    // At least two spaces between columns
    auto value = index.data().toString().size() + 2;
    fieldWidth = std::max(fieldWidth, value);
  });
  return fieldWidth;
}

static void printModel(Model* model, QTextStream &stream) {
  iterate(model, [model, &stream](QModelIndex index) {
    
    auto value = index.data().toString();
    
    stream << (value.isEmpty() ? "-" : value);
    
    // The last column
    if (index.column() == model->columnCount() - 1) {
      stream << endl;
    }
  });
}

static std::vector<int>parseRow(QString line) {
  QStringList list = line.split(" ", QString::SkipEmptyParts);
  
  std::vector<int> row;
  for (auto value : list) {
    row.push_back(value.toInt());
  }
  
  return row;
}

bool MainWindow::writeSolutionToFile(QString fileName) {
  
  QFile file(fileName);
  
  if (!file.open(QIODevice::WriteOnly)) { return false; }
  
  QTextStream out(&file);
  
  out.setFieldWidth(calculateFieldWidth(inputModel));
  
  out << "Problem:" << endl;
  
  printModel(inputModel, out);
  
  out << endl << endl;
  
  out << "Solution:" << endl;
  
  out.setFieldWidth(calculateFieldWidth(outputModel));
  
  printModel(outputModel, out);
  
  out << endl;
  
  return out.status() == QTextStream::Ok;
}

bool MainWindow::readProblemFromFile(QString fileName) {
  
  QFile file(fileName);
  
  if (!file.open(QIODevice::ReadOnly)) { return false; }
  
  QTextStream in(&file);
  
  QString line;
  if (!in.readLineInto(&line) || !line.startsWith("Problem:")) {
    return false;
  }
  
  std::vector<std::vector<int>> problem;
  
  while (in.readLineInto(&line) && !line.startsWith("Solution:")) {
    auto row = parseRow(line);
    if (!row.empty()) {
      problem.push_back(row);
    }
  }
  
  if (in.status() != QTextStream::Ok) {
    return false;
  }
  
  auto comparator = [](const std::vector<int> &lhs,
                       const std::vector<int> &rhs) {
    return lhs.size() < rhs.size();
  };
  
  inputModel->setRowCount(problem.size());
  inputModel->setColumnCount(std::max_element(problem.cbegin(),
                                              problem.cend(),
                                              comparator)->size());
  
  validateInput();
  
  iterate(inputModel, [this, &problem](QModelIndex index) {
    
    std::vector<int>::size_type row = index.row(), column = index.column();
    
    if (row < problem.size() &&
        column < problem[row].size() &&
        problem[row][column] != 0) {
      inputModel->setData(index, problem[row][column]);
    }
  });
  
  clearOutput();
  
  ui->supply_spin->setValue(inputModel->rowCount() - 1);
  ui->demand_spin->setValue(inputModel->columnCount() - 1);
  
  resizeTables();
  
  return true;
}

void MainWindow::loadProblem() {
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Load problem"),
                                                  QString(),
                                                  tr("Text files (*.txt)"));
  
  if (!readProblemFromFile(fileName)) {
    alertOpenFileError(fileName);
  }
}

void MainWindow::saveSolution() {
  
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Save solution"),
                                                  QString(),
                                                  tr("Text files (*.txt)"));
  
  if (fileName.isEmpty()) { return; }
  
  if (!writeSolutionToFile(fileName)) {
    QMessageBox box;
    box.setIcon(QMessageBox::Critical);
    box.setText(tr("Could not save to file %1").arg(fileName));
    box.setWindowTitle(tr("Error"));
    box.exec();
  }
}

void MainWindow::alertOpenFileError(QString fileName) {
  QMessageBox box;
  box.setIcon(QMessageBox::Critical);
  box.setText(tr("Could not read from file %1. "
                 "Maybe the file is not available "
                 "or contains corrupted data.").arg(fileName));
  box.setWindowTitle(tr("Error"));
  box.exec();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  
  if (!event->mimeData()->hasUrls()) { return; }
  
  QUrl file = event->mimeData()->urls().first();
  
  if (file.isLocalFile() && file.fileName().endsWith(".txt")) {
    event->setAccepted(true);
  }
}

void MainWindow::dropEvent(QDropEvent* event) {
  
  QString fileName = event->mimeData()->urls().first().path();
  
  if (!readProblemFromFile(fileName)) {
    alertOpenFileError(fileName);
  }
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About Transportation Problem"),
                     tr("Transportation Problem"));
}

void iterate(Model* model,
             const std::function<void(QModelIndex)> &lambda) {
  for (int row = 0; row < model->rowCount(); ++row) {
    for (int column = 0; column < model->columnCount(); ++column) {
      auto index = model->index(row, column);
      lambda(index);
    }
  }
}
