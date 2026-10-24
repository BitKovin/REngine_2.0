"""Writes every prop prefab to GameData/prefabs/props and a board map with all of them in a row (maps/village_study/props_board.map.json) for looking at."""
import sys
from kit import P
import props

if __name__ == "__main__":
    board = P()
    board.box((-40, -1, -40), (40, 0, 40), "Ground/grass")
    for i, (name, fn) in enumerate(props.PROPS.items()):
        fn().save(f"prefabs/props/{name}.map.json")
        board.inst(f"prefabs/props/{name}.map.json", name, ((i % 6) * 4.5 - 11.25, 0, (i // 6) * 5.5 - 8), 0)
    board.save("maps/village_study/props_board.map.json")
    print(len(props.PROPS), "props")
