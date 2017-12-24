//
//  TransportationProblem.cpp
//  TransportationProblem
//
//  Created by Sergej Jaskiewicz on 04/12/2017.
//  Copyright © 2017 Sergej Jaskiewicz. All rights reserved.
//

#include <limits.h>
#include <assert.h>
#include <string>
#include "TransportationProblem.h"

using namespace TProblem;

Shipment::Shipment(Quantity q,
                   Currency cpu,
                   quantity_index r,
                   quantity_index c): costPerUnit(cpu),
                                      quantity(q),
                                      row(r),
                                      column(c) {}

TransportationProblem::TransportationProblem(std::vector<Quantity> supply,
                                             std::vector<Quantity> demand,
                                             CostMatrix costMatrix):
supply(supply),
demand(demand),
costMatrix(costMatrix),
shipments(ShipmentMatrix(supply.size(),
                          std::vector<Shipment*>(demand.size(), 0))),
_currentSupply(supply),
_currentDemand(demand)
{}

Currency Shipment::cost() const {
  return costPerUnit * quantity;
}

bool TransportationProblem::isBalanced() const {
  return supplyTotal() == demandTotal();
}

bool TransportationProblem::isDegenerate() const {
  return _currentSupply.size() + _currentDemand.size() - 1 != _matrixToList().size();
}

Quantity TransportationProblem::supplyTotal() const {
  return std::accumulate(supply.cbegin(), supply.cend(), 0);
}

Quantity TransportationProblem::demandTotal() const {
  return std::accumulate(demand.cbegin(), demand.cend(), 0);
}

void TransportationProblem::printWithCosts(std::wofstream &stream) {
  prettyPrint<Currency>(stream,
                        costMatrix,
                        [](Currency cost) { return std::to_wstring(cost); });
}

void TransportationProblem::printWithShipments(std::wofstream &stream) {
  
  prettyPrint<Shipment*>(stream,
                         shipments,
                         [](Shipment* shipment) {
                           if (shipment != nullptr) {
                             return std::to_wstring(shipment->quantity);
                           } else {
                             return std::wstring(L"-");
                           }
                         });
}

void TransportationProblem::fixImbalance() {
  
  Quantity supplyTotal = this->supplyTotal();
  Quantity demandTotal = this->demandTotal();
  
  if (supplyTotal > demandTotal) {
    
    demand.push_back(supplyTotal - demandTotal);
    
    for (auto& column : costMatrix) {
      column.push_back(0);
    }
    
    for (auto& column : shipments) {
      column.push_back(nullptr);
    }
    
  } else if (supplyTotal < demandTotal) {
    
    supply.push_back(demandTotal - supplyTotal);
    
    costMatrix.push_back(std::vector<Quantity>(demand.size(), 0));
    shipments.push_back(std::vector<Shipment*>(demand.size(), nullptr));
  }
}

void TransportationProblem::northWestCorner() {
  
  for (quantity_index row = 0, northwest = 0;
       row < _currentSupply.size();
       ++row) {

    for (quantity_index column = northwest;
         column < _currentDemand.size();
         ++column) {
      
      Quantity quantity = std::min(_currentSupply[row], _currentDemand[column]);
      
      if (quantity > 0) {
        shipments[row][column] =
          new Shipment(quantity, costMatrix[row][column], row, column);
        
        _currentSupply[row] -= quantity;
        _currentDemand[column] -= quantity;
        
        if (stateDidChangeCallback) {
          stateDidChangeCallback(this, nullptr);
        }
        
        assert(_currentSupply[row] >= 0);
        
        if (_currentSupply[row] == 0) {
          northwest = column;
          break;
        }
      }
    }
  }
}

void TransportationProblem::steppingStone() {
  
  Currency previousCost = std::numeric_limits<Currency>::quiet_NaN();
  Currency currentCost = totalCost();
    
  while (previousCost != currentCost) {
    Currency maxReduction = 0;
    std::vector<Shipment*> move;
    Shipment* leaving = nullptr;
    
    _fixDegenerateCase();
    
    for (quantity_index row = 0; row < _currentSupply.size(); ++row) {
      for (quantity_index column = 0;
           column < _currentDemand.size();
           ++column) {
        
        if (shipments[row][column] != nullptr) {
          continue;
        }
        
        auto trial = new Shipment(0, costMatrix[row][column], row, column);
        std::vector<Shipment*> path = _getClosedPath(trial);
        
        Currency reduction = 0;
        Quantity lowestQuantity = std::numeric_limits<Quantity>::max();
        Shipment* leavingCandidate = nullptr;
        
        bool plus = true;
        for (auto shipment : path) {
          if (plus) {
            reduction += shipment->costPerUnit;
          } else {
            reduction -= shipment->costPerUnit;
            if (shipment->quantity < lowestQuantity) {
              leavingCandidate = shipment;
              lowestQuantity = shipment->quantity;
            }
          }
          plus = !plus;
        }
        
        if (reduction < maxReduction) {
          move = path;
          leaving = leavingCandidate;
          maxReduction = reduction;
        }
      }
    }
    
    if (!move.empty() && leaving != nullptr) {
      Quantity q = leaving->quantity;
      bool plus = true;
      for (auto shipment : move) {
        
        shipment->quantity += plus ? q : -q;
        
        shipments[shipment->row][shipment->column] = shipment->quantity == 0 ?
                                                      nullptr :
                                                      shipment;
        
        plus = !plus;
      }
      previousCost = currentCost;
      currentCost = totalCost();
      continue;
    } else {
      break;
    }
  }
}

std::list<Shipment*> TransportationProblem::_matrixToList() const {
  
  std::list<Shipment*> list;
  
  for (auto& row : shipments) {
    for (auto shipment : row) {
      if (shipment != nullptr) {
        list.push_back(shipment);
      }
    }
  }
  
  return list;
}

static std::pair<Shipment*, Shipment*>
getNeighbors(Shipment const * shipment,
             std::list<Shipment*> const &list) {
  
  std::pair<Shipment*, Shipment*> neighbors = { nullptr, nullptr };
  
  if (shipment == nullptr) {
    return neighbors;
  }
  
  for (auto current : list) {
    if (current == shipment) { continue; }
    
    if (current->row == shipment->row &&
        neighbors.first == nullptr) {
      
      neighbors.first = current;
      
    } else if (current->column == shipment->column &&
               neighbors.second == nullptr) {
      
      neighbors.second = current;
    }
    
    if (neighbors.first != nullptr && neighbors.second != nullptr) {
      break;
    }
  }
  
  return neighbors;
}

static bool removeElementsWithoutNeighbors(std::list<Shipment*> &list) {
  
  auto beforeCount = list.size();
  
  if (beforeCount == 0) { return false; }
    
  list.remove_if([&list](Shipment * shipment) {
    auto neighbors = getNeighbors(shipment, list);
    return neighbors.first == nullptr || neighbors.second == nullptr;
  });
  
  return beforeCount != list.size();
}

std::vector<Shipment*> TransportationProblem::
  _getClosedPath(TProblem::Shipment* shipment) const {
  
  auto path = _matrixToList();
  
  path.push_front(shipment);
  
  // remove (and keep removing) elements that do not have a
  // vertical AND horizontal neighbor
  while (removeElementsWithoutNeighbors(path));
  
  // place the remaining elements in the correct plus-minus order
  std::vector<Shipment*> stones { path.begin(), path.end() };
  
  auto previousShipment = shipment;
  
  for (quantity_index i = 0; i < stones.size(); ++i) {
    stones[i] = previousShipment;
    auto neighbors = getNeighbors(previousShipment, path);
    previousShipment = (i % 2 == 0) ? neighbors.first : neighbors.second;
  }
  
  return stones;
}

void TransportationProblem::_fixDegenerateCase() {
  
  if (!isDegenerate()) { return; }
  
  for (quantity_index row = 0; row < _currentSupply.size(); ++row) {
    for (quantity_index column = 0; column < _currentDemand.size(); ++column) {

      if (shipments[row][column] != nullptr) { continue; }
      
      auto dummy = new Shipment(0,
                                costMatrix[row][column],
                                row,
                                column);
      
      if (_getClosedPath(dummy).empty()) {
        shipments[row][column] = dummy;
        return;
      } else {
        delete dummy;
      }
    }
  }
}

Currency TransportationProblem::totalCost() const {
  
  Currency accumulator = 0;
  
  for (auto& row : shipments) {
    for (auto shipment : row) {
      if (shipment != nullptr) {
        accumulator += shipment->cost();
      }
    }
  }

  return accumulator;
}

