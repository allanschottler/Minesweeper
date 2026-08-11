#pragma once
#include <optional>
#include <array>
#include <list>
#include <queue>
#include <random>
#include <tuple>
#include <expected>
#include <print>
#include <utility>

namespace ms {

struct Beginner { static constexpr std::size_t ni = 9, nj = 9, mines = 10; };
struct Intermediate { static constexpr std::size_t ni = 16, nj = 16, mines = 40; };
struct Expert { static constexpr std::size_t ni = 30, nj = 16, mines = 99; };

using Cell = std::optional<std::size_t>;

template<class Difficulty, 
         std::size_t NI = Difficulty::ni, 
         std::size_t NJ = Difficulty::nj>
struct Board : std::array<std::array<Cell, NI>, NJ> {
    static constexpr auto ni = NI, nj = NJ;
};

template<class Difficulty>
[[nodiscard]] constexpr auto make_board() -> Board<Difficulty> {
    Board<Difficulty> b;
    
    for(auto&& arr : b)
    for(auto&& cell : arr)
        cell = 0;

    std::list<std::size_t> pool(Difficulty::ni * Difficulty::nj);
    for(auto i(0u); auto& v : pool)
        v = i++;
    
    std::random_device rd; 
    std::mt19937 gen(rd()); 

    int mines{Difficulty::mines};
    while(mines >= 0) {
        std::uniform_int_distribution<std::size_t> distrib(0, pool.size()-1); 
        auto pick = distrib(gen);
        auto row = pick / Difficulty::ni;
        auto col = pick % Difficulty::ni;
        b[row][col] = std::nullopt; //mine
        pool.erase(std::next(pool.begin(), pick));
        mines--;
    }

    for(auto x(0); x<Difficulty::ni; ++x)
    for(auto y(0); y<Difficulty::nj; ++y) {
        if(!b[y][x]) continue;
        std::size_t count{0};
        for(auto i(-1); i<=1; ++i)
        for(auto j(-1); j<=1; ++j) {
            if(i == 0 && j == 0) continue;
            if(x+i<0 || x+i>=Difficulty::ni || 
               y+j<0 || y+j>=Difficulty::nj) continue;
            if(!b[y+j][x+i]) count++;
        }
        b[y][x] = count;
    }    

    return b;
}

using PickedCell = std::tuple<Cell, std::size_t, std::size_t>;

template<class Board>
std::list<PickedCell> mines(const Board& b) {
    std::list<PickedCell> picked;
    for(std::size_t y(0); y<Board::nj; ++y)
    for(std::size_t x(0); x<Board::ni; ++x) {
        const auto& cell = b[y][x];
        if(!cell) picked.emplace_back(cell, x, y);
    }
    return picked;
}

struct Ok { std::list<PickedCell> picked; };
struct Bust { std::list<PickedCell> mines; };

template<class Board>
std::expected<Ok, Bust> pick(const Board& b, std::size_t i, std::size_t j)
{
    std::queue<std::pair<std::size_t, std::size_t>> q;
    std::list<PickedCell> picked;
    std::array<std::array<bool, Board::ni>, Board::nj> visited{};

    q.emplace(i, j);
    visited[j][i] = true;

    while(!q.empty()) {
        const auto [x, y] = q.front();
        q.pop();

        const auto& cell = b[y][x];
        picked.emplace_back(cell, x, y);

        if(!cell) return std::unexpected(Bust{mines(b)});
        if(*cell != 0) continue;

        for(auto dx(-1); dx <= 1; ++dx)
        for(auto dy(-1); dy <= 1; ++dy) {
            if(dx == 0 && dy == 0) continue;

            const auto nx = static_cast<std::size_t>(static_cast<long long>(x) + dx);
            const auto ny = static_cast<std::size_t>(static_cast<long long>(y) + dy);
            if(nx >= Board::ni || ny >= Board::nj) continue;
            if(visited[ny][nx]) continue;

            visited[ny][nx] = true;
            q.emplace(nx, ny);
        }
    }

    return Ok{std::move(picked)};
}

}
