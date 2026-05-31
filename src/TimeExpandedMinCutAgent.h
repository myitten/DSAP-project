#pragma once

#include "Agent.h"
#include <climits>
#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>

class TimeExpandedMinCutAgent : public BaseAgent {
private:
    const int INF_CAP = 999999;

    bool bfs(const std::vector<std::vector<int>>& capacity,
             const std::vector<std::vector<int>>& adj,
             int s, int t, std::vector<int>& parent)
    {
        std::fill(parent.begin(), parent.end(), -1);
        parent[s] = s;
        std::queue<int> q;
        q.push(s);

        while (!q.empty()) {
            int cur = q.front();
            q.pop();

            for (int next : adj[cur]) {
                if (parent[next] == -1 && capacity[cur][next] > 0) {
                    parent[next] = cur;
                    if (next == t) return true;
                    q.push(next);
                }
            }
        }
        return false;
    }

    std::vector<int> calculateTimeMap(int W, int H, const Map& m, const std::deque<std::pair<int, int>>& infectedQueue) {
        std::vector<int> timeMap(W * H, INT_MAX);
        std::queue<std::pair<int, int>> q;

        for (const auto& p : infectedQueue) {
            int idx = p.second * W + p.first;
            timeMap[idx] = 0;
            q.push(p);
        }

        static const int dx[] = {0, 0, -1, 1};
        static const int dy[] = {1, -1, 0, 0};

        while (!q.empty()) {
            auto [cx, cy] = q.front();
            q.pop();
            int curIdx = cy * W + cx;

            for (int i = 0; i < 4; i++) {
                int nx = cx + dx[i];
                int ny = cy + dy[i];
                if (m.isValid(nx, ny) && m.getStatus(nx, ny) != 2) {
                    int nextIdx = ny * W + nx;
                    if (timeMap[nextIdx] == INT_MAX) {
                        timeMap[nextIdx] = timeMap[curIdx] + 1;
                        q.push({nx, ny});
                    }
                }
            }
        }
        return timeMap;
    }

public:
    std::string getAgentName() const override { return "Time-Expanded Min-Cut"; }

    std::pair<int, int> getBlockPoint(const Map& m, const std::deque<std::pair<int, int>>& infectedQueue) override
    {
        int W = m.getWidth();
        int H = m.getHeight();
        int numPixels = W * H;

        int S = numPixels * 2;
        int T = numPixels * 2 + 1;
        int totalNodes = T + 1;

        std::vector<std::vector<int>> capacity(totalNodes, std::vector<int>(totalNodes, 0));
        std::vector<std::vector<int>> adj(totalNodes);


        auto addDirectedEdge = [&](int u, int v, int cap) {
            if (std::find(adj[u].begin(), adj[u].end(), v) == adj[u].end()) adj[u].push_back(v);
            if (std::find(adj[v].begin(), adj[v].end(), u) == adj[v].end()) adj[v].push_back(u);
            capacity[u][v] = cap;
        };

        std::vector<int> timeMap = calculateTimeMap(W, H, m, infectedQueue);

        static const int dx[] = {0, 0, -1, 1};
        static const int dy[] = {1, -1, 0, 0};

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                int i = y * W + x;
                int status = m.getStatus(x, y);

                if (status == 2) continue;

                if (status == 1) {
                    addDirectedEdge(S, i, INF_CAP);
                    addDirectedEdge(i, i + numPixels, INF_CAP);
                } else {
                    int virusReachTime = timeMap[i];

                    if (virusReachTime <= 2) {

                        addDirectedEdge(i, i + numPixels, INF_CAP);
                    } else {

                        addDirectedEdge(i, i + numPixels, 1);
                        addDirectedEdge(i + numPixels, T, INF_CAP);
                    }
                }

                for (int d = 0; d < 4; d++) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (m.isValid(nx, ny) && m.getStatus(nx, ny) != 2) {
                        int j = ny * W + nx;
                        addDirectedEdge(i + numPixels, j, INF_CAP);
                    }
                }
            }
        }

        std::vector<int> parent(totalNodes);
        int maxFlow = 0;
        while (bfs(capacity, adj, S, T, parent)) {
            int pathFlow = INT_MAX;
            for (int v = T; v != S; v = parent[v]) {
                int u = parent[v];
                pathFlow = std::min(pathFlow, capacity[u][v]);
            }
            for (int v = T; v != S; v = parent[v]) {
                int u = parent[v];
                capacity[u][v] -= pathFlow;
                capacity[v][u] += pathFlow;
            }
            maxFlow += pathFlow;
        }

        std::vector<bool> visited(totalNodes, false);
        std::queue<int> q;
        q.push(S);
        visited[S] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : adj[u]) {
                if (!visited[v] && capacity[u][v] > 0) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }

        std::vector<std::pair<int, int>> cutCandidates;
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                if (m.getStatus(x, y) == 0) {
                    int i = y * W + x;
                    if (visited[i] && !visited[i + numPixels] && capacity[i][i + numPixels] == 0) {
                        cutCandidates.push_back({x, y});
                    }
                }
            }
        }

        if (cutCandidates.empty()) return {-1, -1};
        
        std::pair<int, int> bestWall = cutCandidates[0];
        int minDist = INT_MAX;
        for (const auto& wall : cutCandidates) {
            int d = timeMap[wall.second * W + wall.first];
            if (d < minDist) {
                minDist = d;
                bestWall = wall;
            }
        }

        return bestWall;
    }
};
