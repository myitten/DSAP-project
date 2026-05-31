#pragma once

#include "Agent.h"
#include <climits>
#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>

class TimeExpandedMinCutAgent : public BaseAgent {
private:
    // 安全常數：20x20地圖總格數400，9999 已經是絕對的無限大且不會溢位
    const int INF_CAP = 9999;

    // Edmonds-Karp 用的 BFS 尋找增廣路徑
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

    // 計算時空矩陣：病毒在不被阻擋的情況下，第幾回合會到達每一格
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

        // 建立單向邊，防止殘餘網路中的鄰居倒灌
        auto addDirectedEdge = [&](int u, int v, int cap) {
            if (std::find(adj[u].begin(), adj[u].end(), v) == adj[u].end()) adj[u].push_back(v);
            if (std::find(adj[v].begin(), adj[v].end(), u) == adj[v].end()) adj[v].push_back(u);
            capacity[u][v] = cap;
        };

        std::vector<int> timeMap = calculateTimeMap(W, H, m, infectedQueue);

        static const int dx[] = {0, 0, -1, 1};
        static const int dy[] = {1, -1, 0, 0};

        // --- 1. 建構時空圖論網路 ---
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                int i = y * W + x;
                int status = m.getStatus(x, y);

                if (status == 2) continue; // 障礙物不建圖

                if (status == 1) {
                    addDirectedEdge(S, i, INF_CAP);
                    addDirectedEdge(i, i + numPixels, INF_CAP);
                } else {
                    int virusReachTime = timeMap[i];

                    if (virusReachTime <= 2) {
                        // 戰略放棄：時間不夠，這格必定淪陷，將其作為純通道，不連向 T
                        addDirectedEdge(i, i + numPixels, INF_CAP);
                    } else {
                        // 時間充足，允許在這裡築牆（割斷成本1），並將其視為需要保護的領土（連向T）
                        addDirectedEdge(i, i + numPixels, 1);
                        addDirectedEdge(i + numPixels, T, INF_CAP);
                    }
                }

                // 外部邊：只能從我的出口（out）流向鄰居的入口（in）
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

        // --- 2. Edmonds-Karp 執行最大流 ---
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

        // --- 3. 殘餘網路提取最小割集 ---
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
                    // S 走得到入口，但走不到出口，且原本的容量被完全耗盡（=0）
                    if (visited[i] && !visited[i + numPixels] && capacity[i][i + numPixels] == 0) {
                        cutCandidates.push_back({x, y});
                    }
                }
            }
        }

        if (cutCandidates.empty()) return {-1, -1};

        // --- 4. 戰術優先順序 ---
        // 從所有來得及防守的完美割點中，挑選「最迫切需要蓋（離病毒時間最近）」的那面牆先蓋！
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
