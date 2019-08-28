#include <climits>
#include <stack>

#include <engine/logger.h>
#include <engine/map.h>
#include <engine/pathfinding.h>

using namespace engine;

std::array<Types::Point<int32_t>, 4 > PathFinding::sNeighbours = {
    Types::Point<int32_t>( 0, -1 ), Types::Point<int32_t>( 1, 0 ),
    Types::Point<int32_t>( 0, 1 ), Types::Point<int32_t>( -1, 0 )
};


PathFinding::Node::Node(Types::Point<int32_t> pos, Types::Point<int32_t> parent, int32_t gCost, int32_t fCost)
    : pos(pos)
    , parent(parent)
    , gCost(gCost)
    , fCost(fCost)
{

}

std::vector<Types::Point<int32_t>> PathFinding::find(const Map& map, Types::Rect<uint32_t> source, Types::Point<int32_t> dst)
{
    Types::Dimension<> tileDimen = map.getTileDimension();
    Types::Dimension<> sourceDimen(source.width, source.height);

    Types::Point<int32_t> start = Types::Point<int32_t>(std::floor(source.x / tileDimen.width), std::floor(source.y / tileDimen.height));
    Types::Point<int32_t> end = Types::Point<int32_t>(std::floor(dst.x / tileDimen.width), std::floor(dst.y / tileDimen.height));

    if (map.isNodeSolid(end.x, end.y, sourceDimen)) {
        Logger::error("Map") << "Unreachable destination" << dst.x << dst.y;
        return {};
    }

    std::map<std::pair<int32_t, int32_t>, Node> closed;
    std::multimap<int32_t, Node > open;

    int32_t hStart = calcH(start, end);
    open.emplace(hStart, Node(start, {-1, -1}, 0, hStart));

    while (!open.empty()) {
        std::pair<int32_t, Node> p = *open.begin();
        open.erase(open.begin());

        closed.emplace(std::make_pair(p.second.pos.x, p.second.pos.y), p.second);

        if (p.second.pos == end) {
            break;
        }

        for (Types::Point<int32_t> &d: sNeighbours) {
            Types::Point<int32_t> neighbour = p.second.pos + d;
            if (!isValid(neighbour, map)) {
                continue;
            }

            if (closed.find({neighbour.x, neighbour.y}) != closed.end()) {
                continue;
            }

            if (map.isNodeSolid(neighbour.x, neighbour.y, sourceDimen)) {
                closed.emplace(std::make_pair(p.second.pos.x, p.second.pos.y), Node(neighbour, p.second.pos, INT_MAX, INT_MAX));
                continue;
            }

            int32_t gScore = p.second.gCost;
            if (map.isNodePath(neighbour.x, neighbour.y)) {
                gScore += 1;
            } else {
                gScore += 5;
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
                int32_t hCost = calcH(neighbour, end);
                int32_t fScore = (gScore == INT_MAX) ? INT_MAX : gScore + hCost;
                open.emplace(fScore, Node(neighbour, p.second.pos, gScore, fScore));
            }
        }
    }

    // Did we find a valid path?
    if (closed.find({end.x, end.y}) == closed.end()) {
        Logger::error("Map") << "Could not find destination";
        return {};
    }

    // Go through the closed list in reverse order to find the optimal path
    std::stack<Types::Point<int32_t> > paths;
    Node node = closed.at({end.x, end.y});
    while (node.pos != start && node.parent.x != -1) {
        paths.push(node.pos);
        node = closed.at({node.parent.x, node.parent.y});
    }

    std::vector<Types::Point<int32_t> > ret;
    while (!paths.empty()) {
        Types::Dimension<> d = map.getTileDimension();
        Types::Point<int32_t> p = paths.top();
        paths.pop();
        ret.emplace_back(Types::Point<int32_t>(p.x * d.width, p.y * d.height));
    }

    // Since we are looking at nodes above we may not actually end up at the position requested,
    // fix this by replacing the last point with dst
    if (ret.size() > 1) {
        ret.erase(ret.end() - 1);
        ret.emplace_back(dst);
    }

    return ret;
}

int32_t PathFinding::calcH(const Types::Point<int32_t>& p, const Types::Point<int32_t>& end)
{
    return std::max(std::abs(p.x - end.x), std::abs(p.y - end.y));
}

int32_t PathFinding::index(const Types::Point<int32_t>& p, const Map& map)
{
    return p.x + (p.y * map.width());
}

bool PathFinding::isValid(const Types::Point<int32_t> &neighbour, const Map& map)
{
    return (neighbour.x >= 0 && neighbour.x < static_cast<int32_t>(map.width()) && neighbour.y >= 0 && neighbour.y < static_cast<int32_t>(map.height()));
}
