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
#include <algorithm>
#include <list>
#include <utility>
#include <fstream>

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
  void printWithCosts(std::wofstream &stream);
  void printWithShipments(std::wofstream &stream);

  using StateDidChangeCallback = void(TransportationProblem* _Nonnull,
                                      void* _Nullable);

  std::function<StateDidChangeCallback> stateDidChangeCallback = nullptr;

private:
  std::vector<Quantity> _currentSupply;
  std::vector<Quantity> _currentDemand;
  CostMatrix            _cachedCostMatrix;
  void _fixDegenerateCase();
  std::list<Shipment*> _matrixToList() const;
  std::vector<Shipment*> _getClosedPath(Shipment* shipment) const;

  template<typename Entry>
  int _fieldWidth(std::vector<std::vector<Entry>> matrix,
                  std::function<std::wstring(Entry)> printEntry) {

    int fieldWidth = 0;

    for (auto& row : matrix) {
      for (auto entry : row) {
        int width = printEntry(entry).size() + 2;
        fieldWidth = std::max(fieldWidth, width);
      }
    }

    for (auto& q : supply) {
      int width = std::to_string(q).size() + 2;
      fieldWidth = std::max(fieldWidth, width);
    }

    for (auto& q : demand) {
      int width = std::to_string(q).size() + 2;
      fieldWidth = std::max(fieldWidth, width);
    }

    return fieldWidth;
  }

  template<typename Entry>
  void prettyPrint(std::wofstream &stream,
                   std::vector<std::vector<Entry>> matrix,
                   std::function<std::wstring(Entry)> printEntry) {

    using matrix_index = typename std::vector<std::vector<Entry>>::size_type;
    using vector_index = typename std::vector<Entry>::size_type;

    int fieldWidth = _fieldWidth<Entry>(matrix, printEntry);

    for (matrix_index i = 0; i < matrix.size(); ++i) {

      for (auto entry : matrix[i]) {
        stream.width(fieldWidth);
        stream << printEntry(entry);
      }

      stream << L" │ ";
      stream << supply[i] << std::endl;
    }

    for (vector_index i = 0; i <= demand.size() * fieldWidth; ++i) {
      stream << L"─";
    }

    stream << L"┘" << std::endl;

    for (auto q : demand) {
      stream.width(fieldWidth);
      stream << q;
    }

    stream << std::endl << std::endl;
  }
};
} /* End of namespace TProblem */

#endif
