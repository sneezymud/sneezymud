from peewee import MySQLDatabase, Model, CharField, IntegerField, ForeignKeyField
import svgwrite

db = MySQLDatabase("sneezy", host="localhost", port=3306, user="root", password="")


class Room(Model):
    vnum = IntegerField(primary_key=True)
    name = CharField()
    zone = IntegerField()
    sector = IntegerField()

    class Meta:
        database = db


class RoomExit(Model):
    vnum = IntegerField(primary_key=True)
    author = ForeignKeyField(Room, backref="exits", db_column="vnum")
    direction = IntegerField()
    destination = IntegerField()

    class Meta:
        database = db


class Coord:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def translate(self, dx, dy, dz):
        return Coord(self.x + dx, self.y + dy, self.z + dz)

    def clone(self):
        return Coord(self.x, self.y, self.z)


class ParsedRoom:
    def __init__(self, vnum, name, coord, sector):
        self.vnum = vnum
        self.name = name
        self.coord = coord

        # Colors extracted from `sneezymud/code/code/sys/db.cc`. Except for Market Square (vnum
        # 238), which just marks Market Square. Extracted colors may not be 100% correct.
        if vnum == 238:
            self.color = "orange"
        elif sector == 0:
            self.color = "magenta"
        elif sector == 1:
            self.color = "white"
        elif sector == 2:
            self.color = "cyan"
        elif sector == 3:
            self.color = "white"
        elif sector == 4:
            self.color = "black"
        elif sector == 5:
            self.color = "black"
        elif sector == 6:
            self.color = "green"
        elif sector == 7:
            self.color = "blue"
        elif sector == 8:
            self.color = "cyan"
        elif sector == 9:
            self.color = "cyan"
        elif sector == 10:
            self.color = "magenta"
        elif sector == 11:
            self.color = "cyan"
        elif sector == 12:
            self.color = "magenta"
        elif sector == 13:
            self.color = "cyan"
        elif sector == 14:
            # Artic Atmosphere
            self.color = "cyan"
        elif sector == 15:
            self.color = "magenta"
        elif sector == 16:
            self.color = "magenta"
        elif sector == 17:
            # Plains
            self.color = "green"
        elif sector == 18:
            # Temperate City
            self.color = "magenta"
        elif sector == 19:
            # Temperate Road
            self.color = "magenta"
        elif sector == 20:
            self.color = "green"
        elif sector == 21:
            self.color = "black"
        elif sector == 22:
            self.color = "green"
        elif sector == 23:
            self.color = "green"
        elif sector == 24:
            self.color = "magenta"
        elif sector == 25:
            self.color = "cyan"
        elif sector == 26:
            self.color = "blue"
        elif sector == 27:
            # Temperate Underwater
            self.color = "cyan"
        elif sector == 28:
            self.color = "black"
        elif sector == 29:
            self.color = "green"
        elif sector == 30:
            self.color = "green"
        elif sector == 31:
            self.color = "green"
        elif sector == 32:
            self.color = "yellow"
        elif sector == 33:
            self.color = "yellow"
        elif sector == 34:
            self.color = "green"
        elif sector == 35:
            self.color = "green"
        elif sector == 36:
            self.color = "green"
        elif sector == 37:
            # Jungle
            self.color = "magenta"
        elif sector == 38:
            self.color = "green"
        elif sector == 39:
            self.color = "red"
        elif sector == 40:
            self.color = "magenta"
        elif sector == 41:
            self.color = "yellow"
        elif sector == 42:
            self.color = "green"
        elif sector == 43:
            self.color = "blue"
        elif sector == 44:
            self.color = "cyan"
        elif sector == 45:
            self.color = "blue"
        elif sector == 46:
            # Tropical Beach
            self.color = "magenta"
        elif sector == 47:
            self.color = "magenta"
        elif sector == 48:
            self.color = "magenta"
        elif sector == 49:
            self.color = "magenta"
        elif sector == 50:
            self.color = "magenta"
        elif sector == 51:
            self.color = "cyan"
        elif sector == 52:
            self.color = "black"
        elif sector == 53:
            # Fire
            self.color = "yellow"
        elif sector == 54:
            self.color = "red"
        elif sector == 55:
            self.color = "yellow"
        elif sector == 56:
            self.color = "black"
        else:
            self.color = "magenta"


class ParsedExit:
    def __init__(self, start_coord, end_coord):
        self.start_coord = start_coord
        self.end_coord = end_coord


def position_rooms():
    print("Positioning rooms...")

    db.connect()

    parsed_rooms = []
    parsed_exits = []
    to_explore = []
    explored = set()

    coord = Coord(0, 0, 0)

    # Start exploring from Market Square
    to_explore.append((238, coord))
    explored.add(238)

    while len(to_explore) > 0:
        (room_vnum, coord) = to_explore.pop()

        room = Room.get(Room.vnum == room_vnum)

        parsed_rooms.append(ParsedRoom(room_vnum, room.name, coord, room.sector))

        for exit in room.exits:
            if exit.destination >= 0 and exit.destination not in explored:
                explored.add(exit.destination)

                if exit.direction == 0:
                    # N
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(0, -1, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(0, -1, 0)))
                elif exit.direction == 1:
                    # E
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(1, 0, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(1, 0, 0)))
                elif exit.direction == 2:
                    # S
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(0, 1, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(0, 1, 0)))
                elif exit.direction == 3:
                    # W
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(-1, 0, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(-1, 0, 0)))
                elif exit.direction == 4:
                    # U
                    to_explore.append((exit.destination, coord.translate(0, 0, 1)))
                elif exit.direction == 5:
                    # D
                    to_explore.append((exit.destination, coord.translate(0, 0, -1)))
                elif exit.direction == 6:
                    # NE
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(1, -1, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(1, -1, 0)))
                elif exit.direction == 7:
                    # NW
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(-1, -1, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(-1, -1, 0)))
                elif exit.direction == 8:
                    # SE
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(1, 1, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(1, 1, 0)))
                elif exit.direction == 9:
                    # SW
                    parsed_exits.append(
                        ParsedExit(coord.clone(), coord.translate(-1, 1, 0))
                    )
                    to_explore.append((exit.destination, coord.translate(-1, 1, 0)))

    db.close()

    return (parsed_rooms, parsed_exits)


def normalize(rooms, exits):
    print("Normalizing rooms and exits...")

    min_x = 0
    min_y = 0
    max_x = 0
    max_y = 0

    for room in rooms:
        if room.coord.x < min_x:
            min_x = room.coord.x

        if room.coord.y < min_y:
            min_y = room.coord.y

        if room.coord.x > max_x:
            max_x = room.coord.x

        if room.coord.y > max_y:
            max_y = room.coord.y

    print(
        f"Normalizing coordinates with smallest X: {min_x}, Y: {min_y}, largest X: {max_x}, Y: {max_y}"
    )

    for room in rooms:
        room.coord = room.coord.translate(-min_x, -min_y, 0)

    for exit in exits:
        exit.start_coord = exit.start_coord.translate(-min_x, -min_y, 0)
        exit.end_coord = exit.end_coord.translate(-min_x, -min_y, 0)

    return (min_x, min_y, max_x, max_y)


EXIT_OFFSET = 5
ROOM_SIZE = 9
PATH_SIZE = 6


def draw_exit(svg, exit):
    line = svg.line(
        start=(
            exit.start_coord.x * (ROOM_SIZE + PATH_SIZE) + EXIT_OFFSET,
            exit.start_coord.y * (ROOM_SIZE + PATH_SIZE) + EXIT_OFFSET,
        ),
        end=(
            exit.end_coord.x * (ROOM_SIZE + PATH_SIZE) + EXIT_OFFSET,
            exit.end_coord.y * (ROOM_SIZE + PATH_SIZE) + EXIT_OFFSET,
        ),
        stroke="black",
        stroke_width="1",
    )
    svg.add(line)


def draw_room(svg, room):
    rect = svg.rect(
        insert=(
            room.coord.x * (ROOM_SIZE + PATH_SIZE),
            room.coord.y * (ROOM_SIZE + PATH_SIZE),
        ),
        size=(ROOM_SIZE, ROOM_SIZE),
        stroke="black",
        stroke_width="1",
        fill=room.color,
    )
    rect.set_desc(title=f"{room.name} ({room.vnum})")
    svg.add(rect)


def draw_map(rooms, exits, map_width, map_height):
    print("Drawing map...")

    svg = svgwrite.Drawing(
        "map.svg",
        size=(
            f"{map_width * (ROOM_SIZE + PATH_SIZE)}px",
            f"{map_height * (ROOM_SIZE + PATH_SIZE)}px",
        ),
    )

    for exit in exits:
        draw_exit(svg, exit)

    drawn = {}

    for room in rooms:
        level = drawn.get((room.coord.x, room.coord.y))

        if level is None:
            draw_room(svg, room)
        elif abs(room.coord.z) < abs(level):
            draw_room(svg, room)

        drawn[(room.coord.x, room.coord.y)] = room.coord.z

    svg.save()


def main():
    (rooms, exits) = position_rooms()
    (min_x, min_y, max_x, max_y) = normalize(rooms, exits)
    draw_map(rooms, exits, max_x - min_x + 1, max_y - min_y + 1)


if __name__ == "__main__":
    main()
