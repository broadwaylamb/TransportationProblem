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

TransportationProblem::TransportationProblem(std::vector<Quantity> source,
                                             std::vector<Quantity> destination,
                                             CostMatrix cost) {
  
  Quantity source_total = std::accumulate(source.cbegin(),
                                          source.cend(),
                                          0);
  Quantity dest_total = std::accumulate(destination.cbegin(),
                                        destination.cend(),
                                        0);
  
  // Fix imbalance
  if (source_total > dest_total) {
    destination.push_back(source_total - dest_total);
  } else if (source_total < dest_total) {
    source.push_back(dest_total - source_total);
  }
  
  _src = source;
  _dest = destination;
  _costMatrix = cost;
  _shipments = ShipmentMatrix(source.size(),
                              std::vector<Shipment*>(destination.size()));
}

std::vector<Quantity> TransportationProblem::source() const {
  return _src;
}

std::vector<Quantity> TransportationProblem::destination() const {
  return _dest;
}

TransportationProblem::CostMatrix TransportationProblem::costMatrix() const {
  return _costMatrix;
}

TransportationProblem::ShipmentMatrix TransportationProblem::shipments() const {
  return _shipments;
}

void TransportationProblem::northWestCorner() {
  
  for (quantity_index row = 0, northwest = 0; row < _src.size(); ++row) {
    for (quantity_index column = northwest; column < _dest.size(); ++column) {
      
      Quantity quantity = std::min(_src[row], _dest[column]);
      
      if (quantity > 0) {
        _shipments[row][column] =
        new Shipment(quantity, _costMatrix[row][column], row, column);
        
        _src[row] -= quantity;
        _dest[column] -= quantity;
        
        if (stateDidChangeCallback) {
          stateDidChangeCallback(this, nullptr);
        }
        
        assert(_src[row] >= 0);
        
        if (_src[row] == 0) {
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
  
  for (quantity_index row = 0; row < _src.size(); ++row) {
    for (quantity_index column = 0; column < _dest.size(); ++column) {
      
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
  
  for (auto current : list) {
    if (current == shipment) { continue; }
    
    if (current->row == shipment->row && !neighbors.first) {
      neighbors.first = current;
    } else if (current->column == shipment->column && neighbors.second) {
      neighbors.second = current;
    }
    
    if (neighbors.first && neighbors.second) {
      break;
    }
  }
  
  return neighbors;
}

static bool removeElementsWithoutNeighbors(std::list<Shipment*> &list) {
  
  auto beforeCount = list.size();
  list.remove_if([&list](Shipment * shipment){
    auto neighbors = getNeighbors(shipment, list);
    return neighbors.first == nullptr && neighbors.second == nullptr;
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
  
  return  stones;
}

void TransportationProblem::_fixDegenerateCase() {
  
  auto eps = std::numeric_limits<Quantity>::min();
  
  if (_src.size() + _dest.size() - 1 == _matrixToList().size()) { return; }
  
  for (quantity_index row = 0; row < _src.size(); ++row) {
    for (quantity_index column = 0; column < _dest.size(); ++column) {
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
