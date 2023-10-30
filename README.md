# Botzilla

This is my bot for the [infinibattle 2023](https://infi.nl/nieuws/infinibattle-23/). It uses a [Monte-Carlo tree search (MCTS)](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) with ε-greedy exploration to play Barrage stratego.

To evaluate non-terminal nodes, the following evaluation function $f$ is used to evaluate a state $s$ (assuming the bot is player 0). For a terminal state $s$, the value of $f(s)$ is 1 if the bot has won, 0 if it has lost, and 0.5 if it's a draw. For a nonterminal state $s$:

$$ f(s) = 1 - (1 - (\frac{N_0}{7 N_1} + \frac{0.35}{d_f}))^8 $$

Here, $N_0, N_1$ are the number of non-flag units alive for the bot and the enemy, respectively. $d_f$ is the Manhattan distance from the closest piece of the to the flag of the enemy.

This project is written in C++ and should work on both Windows (using Visual Studio) and Linux.


## Pseudocode description of the algorithm

```
state: contains the state of the current game

for i in range(0, number_of_iterations):
    s = state
    
    fill in the unknown pieces of enemy in s in a way that is consistent
        with the information we have (the moved pieces and revealed pieces)

    root = Node()
    current = root
    
    # selection
    while True:
        possible_moves = enumerate_moves(s)
        
        with probability ε:
            pick a random move from possible_moves
        else:
            from the intersection of current.children and possible_moves,
                pick the move that has the optimal value (highest for the bot, lowest for the enemy)
        
        if chosen_move not in current.children:
            node = Node()
            current.children.append(node)
            current = node
            break
        
        s.do_move(chosen_move)
        current = current.children[chosen_move]
        
        if s is a terminal state:
            break
    
    # evaluation
    if s is a terminal state:
        if player 0 has won:
            val = 1
        elsif player 1 has won:
            val = 0
        else if draw:
            val = 0.5
    else:
        val = evaluate(s)
    
    # backpropagate and recalculate the statistics back to the root
    while current != None:
        current.nom += val
        current.denom += 1
        current.value = current.nom / current.denom
        current = current.parent

pick the move from root.children that has the highest value
```

## Architecture

The bot uses a class `State` to manage the state. The state contains a 10x10 board that is represented by 100 bytes. A value of 255 represents an empty cell, a value of 254 represents water. Every value `v` with `0 <= v <= 15` is an index into the array of pieces. The indices 0..7 are for player 0, the indices 8..15 are for player 1. The array of pieces contains the rank of the piece in the lower 3 bits. The 4th bit (bit 3) is the move bit, which is set if the piece has moved. The 5th bit is the reveal bit, which is set if the piece has moved (I ended up not using this bit anywhere).

The `State` class has two methods, `do_move` and `reveal`. Only pieces which are revealed can attack or be attacked.

There are three executables that can be produced: `wrapper`, `tournament`, and `situation`.
  - The `wrapper` is used to play against other bots via a runner. The communication with the runner happens via JSON over standard input/output.
  - The `tournament` is used to test how multiple bots perform against eachother. It is a multithreaded program that lets all the bots play a bunch of matches against eachother, keeping track of the number of wins/draws/losses for each bot.
  - The `situation` is used to play a specific situation. I used it to debug situations where the bot showed weird behavior.


## Known problems

Due to the ε-greedy exploration strategy, the value of terminal positions dissipates quickly over the number of moves. The bot will sometimes happily let a defending piece walk by an attacking piece even though there is no other piece close enough to defend the flag.

A possible solution would be to use [Upper Confidence bounds applied to Trees (UCT)](https://www.chessprogramming.org/UCT). I tried implementing this, but it seemed to hang (or it was extremely slow), so I stuck with ε-greedy.
