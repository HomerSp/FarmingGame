#include <cfloat>
#include <climits>
#include <queue>
#include <stack>

#include <engine/logger.h>
#include <engine/map.h>
#include <engine/pathfinding.h>

using namespace engine;

std::array<Types::Point<int32_t>, 8 > PathFinding::sNeighbours = {
    Types::Point<int32_t>( -1, -1 ), Types::Point<int32_t>(  1, -1 ),
    Types::Point<int32_t>( -1,  1 ), Types::Point<int32_t>(  1,  1 ),
    Types::Point<int32_t>(  0, -1 ), Types::Point<int32_t>( -1,  0 ),
    Types::Point<int32_t>(  0,  1 ), Types::Point<int32_t>(  1,  0 ),
};


PathFinding::Node::Node(Types::Point<int32_t> pos, Types::Point<int32_t> parent, int32_t gCost, int32_t fCost)
    : pos(pos)
    , parent(parent)
    , gCost(gCost)
    , fCost(fCost)
{

}

std::vector<Types::Point<int32_t>> PathFinding::find(std::shared_ptr<Map>& map, Types::Rect<uint32_t> source, Types::Point<int32_t> dst)
{
    Types::Dimension<> tileDimen = map->getTileDimension();
    Types::Dimension<> sourceDimen(source.width, source.height);

    if (map->isSolid(dst.x * tileDimen.width, dst.y * tileDimen.height, sourceDimen)) {
        Logger::error() << "Unreachable destination" << dst.x << dst.y;
        return {};
    }

    Types::Point<int32_t> start = Types::Point<int32_t>(source.x, source.y);

    std::unordered_map<int32_t, Node> closed;
    std::multimap<int32_t, Node > open;

    int32_t hStart = calcH(start, dst);
    open.emplace(hStart, Node(start, {-1, -1}, 0, hStart));

    while (!open.empty()) {
        std::pair<int32_t, Node> p = *open.begin();
        open.erase(open.begin());

        closed.emplace(index(p.second.pos, map), p.second);

        if (p.second.pos == dst) {
            break;
        }

        for (Types::Point<int32_t> &d: sNeighbours) {
            Types::Point<int32_t> neighbour = p.second.pos + d;
            if (!isValid(neighbour, map)) {
                continue;
            }

            int32_t in = index(neighbour, map);
            if (closed.find(in) != closed.end()) {
                continue;
            }

            int32_t gScore = p.second.gCost;

            if (map->isSolid(neighbour.x * tileDimen.width, neighbour.y * tileDimen.height, sourceDimen)) {
                gScore = INT_MAX;
            } else if (map->isPath(neighbour.x, neighbour.y)) {
                gScore += 1;
            } else {
                gScore += 3;
            }

            bool addOpen = true;
            for (auto it = open.begin(); it != open.end(); it++) {
                if (it->second.pos == neighbour) {
                    if (gScore < it->second.gCost) {
                        open.erase(it);
                    } else {
                        addOpen = false;
                    }

                    break;
                }
            }

            if (addOpen) {
                int32_t hCost = calcH(neighbour, dst);
                int32_t fScore = (gScore == INT_MAX) ? INT_MAX : gScore + hCost;
                open.emplace(fScore, Node(neighbour, p.second.pos, gScore, fScore));
            }
        }
    }

    int32_t dstIndex = index(dst, map);
    if (closed.find(dstIndex) == closed.end()) {
        Logger::debug() << "Could not find destination";
        return {};
    }

    std::stack<Types::Point<int32_t> > paths;

    Node node = closed.at(dstIndex);
    while (node.pos != start && node.parent.x != -1) {
        paths.push(node.pos);
        node = closed.at(index(node.parent, map));
    }

    std::vector<Types::Point<int32_t> > ret;
    while (!paths.empty()) {
        Types::Point<int32_t> point = paths.top();
        paths.pop();

        // Simplify the list by only adding points where the direction changes
        if (ret.size() > 1) {
            Types::Point<int32_t> &retPt = ret.back();
            if (point.x == retPt.x && point.y != retPt.y) {
                retPt.y = point.y;
                continue;
            }

            if (point.y == retPt.y && point.x != retPt.x) {
                retPt.x = point.x;
                continue;
            }
        }

        ret.emplace_back(point);
    }

    return ret;
}

int32_t PathFinding::calcH(const Types::Point<int32_t>& p, const Types::Point<int32_t>& end)
{
    return std::max(std::abs(p.x - end.x), std::abs(p.y - end.y));
}

int32_t PathFinding::index(const Types::Point<int32_t>& p, std::shared_ptr<Map>& map)
{
    return p.x + (p.y * map->width());
}

bool PathFinding::isValid(const Types::Point<int32_t> &neighbour, std::shared_ptr<Map>& map)
{
    return (neighbour.x >= 0 && neighbour.x < static_cast<int32_t>(map->width()) && neighbour.y >= 0 && neighbour.y < static_cast<int32_t>(map->height()));
}
