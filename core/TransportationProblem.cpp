//
//  TransportationProblem.cpp
//  TransportationProblem
//
//  Created by Sergej Jaskiewicz on 04/12/2017.
//  Copyright © 2017 Sergej Jaskiewicz. All rights reserved.
//

#include <limits.h>
#include <assert.h>
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
                                             CostMatrix cost) {
  
  Quantity supply_total = std::accumulate(supply.cbegin(),
                                          supply.cend(),
                                          0);
  Quantity demand_total = std::accumulate(demand.cbegin(),
                                          demand.cend(),
                                          0);
  
  // Fix imbalance
  if (supply_total > demand_total) {
    
    demand.push_back(supply_total - demand_total);
    
    for (auto& column : cost) {
      column.push_back(0);
    }
    
  } else if (supply_total < demand_total) {
    
    supply.push_back(demand_total - supply_total);
    
    cost.push_back(std::vector<Quantity>(demand.size(), 0));
  }
  
  _supply = supply;
  _demand = demand;
  _costMatrix = cost;
  _shipments = ShipmentMatrix(supply.size(),
                              std::vector<Shipment*>(demand.size(), 0));
}

std::vector<Quantity> TransportationProblem::supply() const {
  return _supply;
}

std::vector<Quantity> TransportationProblem::demand() const {
  return _demand;
}

TransportationProblem::CostMatrix TransportationProblem::costMatrix() const {
  return _costMatrix;
}

TransportationProblem::ShipmentMatrix TransportationProblem::shipments() const {
  return _shipments;
}

void TransportationProblem::northWestCorner() {
  
  for (quantity_index row = 0, northwest = 0; row < _supply.size(); ++row) {
    for (quantity_index column = northwest; column < _demand.size(); ++column) {
      
      Quantity quantity = std::min(_supply[row], _demand[column]);
      
      if (quantity > 0) {
        _shipments[row][column] =
        new Shipment(quantity, _costMatrix[row][column], row, column);
        
        _supply[row] -= quantity;
        _demand[column] -= quantity;
        
        if (stateDidChangeCallback) {
          stateDidChangeCallback(this, nullptr);
        }
        
        assert(_supply[row] >= 0);
        
        if (_supply[row] == 0) {
          northwest = column;
          break;
        }
      }
    }
  }
}

void TransportationProblem::steppingStone() {
  
  Currency maxReduction = 0;
  std::vector<Shipment*> move;
  Shipment* leaving = nullptr;
  
  _fixDegenerateCase();
  
  for (quantity_index row = 0; row < _supply.size(); ++row) {
    for (quantity_index column = 0; column < _demand.size(); ++column) {
      
      if (_shipments[row][column] != nullptr) {
        continue;
      }
      
      auto trial = new Shipment(0, _costMatrix[row][column], row, column);
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
      
      _shipments[shipment->row][shipment->column] = shipment->quantity == 0 ?
                                                      nullptr :
                                                      shipment;
      
      plus = !plus;
    }
    
    // FIXME: Remove recursive call
    
    steppingStone();
  }
}

std::list<Shipment*> TransportationProblem::_matrixToList() const {
  
  std::list<Shipment*> list;
  
  for (auto& row : _shipments) {
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
  _getClosedPath(TProblem::Shipment * _Nonnull shipment) const {
  
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
  
  auto eps = std::numeric_limits<Quantity>::min();
  
  if (_supply.size() + _demand.size() - 1 == _matrixToList().size()) { return; }
  
  for (quantity_index row = 0; row < _supply.size(); ++row) {
    for (quantity_index column = 0; column < _demand.size(); ++column) {
      if (_shipments[row][column] != nullptr) { continue; }
      
      auto dummy = new Shipment(eps,
                                _costMatrix[row][column],
                                row,
                                column);
      
      if (_getClosedPath(dummy).empty()) {
        _shipments[row][column] = dummy;
        return;
      }
    }
  }
}
