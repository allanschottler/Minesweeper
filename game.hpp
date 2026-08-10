#pragma once
#include <optional>
#include <array>
#include <list>
#include <random>
#include <print>

namespace ms {

struct Beginner { static constexpr std::size_t ni = 9, nj = 9, mines = 10; };
struct Intermediate { static constexpr std::size_t ni = 16, nj = 16, mines = 40; };
struct Expert { static constexpr std::size_t ni = 30, nj = 16, mines = 99; };

using Pos = std::optional<std::size_t>;

template<class Difficulty, 
         std::size_t NI = Difficulty::ni, 
         std::size_t NJ = Difficulty::nj>
struct Board : std::array<std::array<Pos, NI>, NJ> {
    static constexpr auto ni = NI, nj = NJ;
};

template<class Difficulty>
[[nodiscard]] constexpr auto make_board() -> Board<Difficulty> {
    Board<Difficulty> b;
    
    for(auto&& arr : b)
    for(auto&& pos : arr)
        pos = 0;

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
        b[col][row] = std::nullopt; //mine
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

template<class Board>
constexpr void print_board(const Board& b) {
    for(auto&& arr : b) {
        for(auto&& pos : arr) {
            if(pos) std::print("{} ", *pos);
            else std::print("X ");
        }
        std::println();
    }
}

}
