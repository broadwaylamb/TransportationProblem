//
//  TransportationProblem.h
//  TransportationProblem
//
//  Created by Sergej Jaskiewicz on 04/12/2017.
//  Copyright © 2017 Sergej Jaskiewicz. All rights reserved.
//

#ifndef TransportationProblem_
#define TransportationProblem_

#include <vector>
#include <numeric>
#include <functional>
#include <list>
#include <utility>
#include <iostream>

namespace TProblem {
  
using Quantity = int;
using Currency = int;
using quantity_index = std::vector<Quantity>::size_type;

struct Shipment {
  Currency       costPerUnit;
  Quantity       quantity;
  quantity_index row;
  quantity_index column;
  
  Currency cost() const;
  
  Shipment(Quantity q, Currency cpu, quantity_index r, quantity_index c);
};

class TransportationProblem {
public:
  
  using CostMatrix = std::vector<std::vector<Currency>>;
  using ShipmentMatrix = std::vector<std::vector<Shipment*>>;
  
  TransportationProblem(std::vector<Quantity> supply,
                        std::vector<Quantity> demand,
                        CostMatrix cost);
  
  std::vector<Quantity> supply;
  std::vector<Quantity> demand;
  CostMatrix            costMatrix;
  ShipmentMatrix        shipments;
  
  // Algorithm steps
  
  void fixImbalance();
  void northWestCorner();
  void steppingStone();
  
  // Calculating stuff
  bool     isBalanced() const;
  bool     isDegenerate() const;
  Quantity supplyTotal() const;
  Quantity demandTotal() const;
  Currency totalCost() const;
    
  // Printing
  void printShipments(std::ostream &stream);
  
  using StateDidChangeCallback = void(TransportationProblem* _Nonnull,
                                      void* _Nullable);
  
  std::function<StateDidChangeCallback> stateDidChangeCallback = nullptr;
  
private:
  void _fixDegenerateCase();
  std::list<Shipment*> _matrixToList() const;
  std::vector<Shipment*> _getClosedPath(Shipment* _Nonnull shipment) const;
};
} /* End of namespace TProblem */

#endif
