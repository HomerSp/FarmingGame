#pragma once

#include <list>

#include <engine/types.h>

namespace engine {

class Map;

class PathFinding {
private:
    class Node {
    public:
        Node(Types::Point<int32_t> pos, Types::Point<int32_t> parent, int32_t gCost, int32_t fCost);

        Types::Point<int32_t> pos, parent;
        int32_t gCost, fCost;
    };

public:
    static std::vector<Types::Point<int32_t>> find(std::shared_ptr<Map>& map, Types::Rect<uint32_t> source, Types::Point<int32_t> dst);

private:
    PathFinding() = default;

    static inline int32_t calcH(const Types::Point<int32_t>& p, const Types::Point<int32_t>& end);
    static inline int32_t index(const Types::Point<int32_t>& p, std::shared_ptr<Map>& map);
    static inline bool isValid(const Types::Point<int32_t>& neighbour, std::shared_ptr<Map>& map);

    static std::array<Types::Point<int32_t>, 4 > sNeighbours;
};
} 
