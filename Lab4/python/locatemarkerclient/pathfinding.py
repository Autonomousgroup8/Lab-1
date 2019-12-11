# import timeit
# code_to_test = """

from operator import add
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from numpy.linalg import norm

def PathFinding(grid):
    # Setup array with distance data
    height = grid.shape[0]
    width = grid.shape[1]

    pathdata = np.zeros((height, width), dtype=int)

    # Distance calculation
    # Setup variable for dijkstra algorithm
    unexplored = [start_coords]
    directions = [[-1, 0], [1, 0], [0, -1], [0, 1]]

    while unexplored:  # While unexplored is not empty
        current_cell = unexplored[0]  # Select an unexplored cell
        for direction in directions:
            neighbour = list(map(add, current_cell, direction))  # Get a neighbour of the current cell

            # If this neighbour a valid cell
            if 0 <= neighbour[0] < height and 0 <= neighbour[1] < width:
                grid_element = grid[neighbour[0]][neighbour[1]]  # Get type of neighbour cell
                pathdata_neighbour = pathdata[neighbour[0]][neighbour[1]]  # Get distance of neighbour cell
                pathdata_current = pathdata[current_cell[0]][current_cell[1]]  # Get distance of current cell

                # If neighbour cell is empty or finish and its distance is zero or larger than current cell distance
                if (grid_element == empty or grid_element == finish) and (
                        pathdata_neighbour > pathdata_current + 1 or pathdata_neighbour == empty):
                    unexplored.append(neighbour)  # Add neighbour to unexplored list
                    pathdata[neighbour[0]][
                        neighbour[1]] = pathdata_current + 1  # Set distance of neighbour to current distance + 1
        unexplored.remove(current_cell)  # Remove explored cell

    # Pathfinding
    # Start at the finish with no direction
    current_cell = finish_coords
    current_direction = []
    instructions = []

    while pathdata[current_cell[0]][current_cell[1]] > 0:
        possible_directions = []

        for direction in directions:
            neighbour = list(map(add, current_cell, direction))  # Get a neighbour of the current cell

            # If this neighbour a valid cell
            if 0 <= neighbour[0] < height and 0 <= neighbour[1] < width:
                grid_element = grid[neighbour[0]][neighbour[1]]  # Get type of neighbour cell
                pathdata_neighbour = pathdata[neighbour[0]][neighbour[1]]  # Get distance of neighbour cell
                pathdata_current = pathdata[current_cell[0]][current_cell[1]]  # Get distance of current cell

                # If neighbour cell is empty and its distance is one smaller than current cell distance
                if (grid_element == empty or grid_element == start) and (pathdata_neighbour == pathdata_current - 1):
                    possible_directions.append(direction)  # Save possible direction

        # If current direction is a possible direction continue in said direction
        if current_direction in possible_directions:
            next_cell = list(map(add, current_cell, current_direction))  # Compute  next cell
            grid[next_cell[0], next_cell[1]] = path  # Make it a path
            instructions[-1] = [instructions[-1][0],
                                str(int(instructions[-1][1]) + 1)]  # Increment fwd in instruction list
        else:
            # If no current direction is set
            if len(current_direction) == 0:
                instructions.append(["F", "1"])  # Add fwd to instruction list
            else:
                v0 = -np.array(current_direction)  # Direction vector1
                v1 = np.array(possible_directions[0]) - np.array(current_direction)  # Direction vector2
                angle = int(np.rad2deg(-2 * np.math.atan2(np.linalg.det([v0, v1]),
                                                          np.dot(v0, v1))))  # Compute the angle between the vectors

                instructions.append(["T", str(angle)])  # Add steering to instruction list
                instructions.append(["F", "1"])  # Add fwd to instruction list

            current_direction = possible_directions[0]  # Select the first of the possibilities
            next_cell = list(map(add, current_cell, possible_directions[0]))  # Compute  next cell
            grid[next_cell[0], next_cell[1]] = path  # Make it a path

        current_cell = next_cell  # Go to next cell

    instructions.reverse()  # Reverse list

    return instructions, pathdata, grid

width = 10
height = 5

# Define elements in grid
empty = 0
start = 1
finish = 2
wall = 3
path = 4

# Setup start and finish coordinates
start_coords = [0, 0]
finish_coords = [height - 1, width - 1]

# Setup grid
grid = np.zeros((height, width), dtype=int)
grid[start_coords[0], start_coords[1]] = start
grid[finish_coords[0], finish_coords[1]] = finish
grid[1:4,7] = wall
grid[0,1:5] = wall
grid[4,3] = wall
grid[1,4] = wall
grid[3,9] = wall
grid[3,3:8] = wall

instructions, pathdata, grid = PathFinding(grid)

# """
# print(timeit.timeit(code_to_test, number=100)/100) # 10x5 0.005220439069999999 100x150 0.21038532765999998


# Visualisation
grid[start_coords[0], start_coords[1]] = start
grid[finish_coords[0], finish_coords[1]] = finish
_, ax = plt.subplots()
cmap = mpl.colors.ListedColormap(['white','red','black','green'])
bounds=[0,1,3,4,5]
norm = mpl.colors.BoundaryNorm(bounds, cmap.N)

for ([i, j],val) in np.ndenumerate(pathdata):
    if grid[i,j] != empty and grid[i,j] != path:
        text = ax.text(j, i, pathdata[i, j], ha="center", va="center", color="w", size="larger")
        text = ax.text(j - 0.45, i + 0.45, grid[i, j], ha="left", va="bottom", color="w", size="smaller")
    else:
        text = ax.text(j, i, pathdata[i, j], ha="center", va="center", color="k", size="larger")
        text = ax.text(j - 0.45, i + 0.45, grid[i, j], ha="left", va="bottom", color="k", size="smaller")

for i in range(len(instructions)):
    text = ax.text(-1.6, i*0.5-0.5, instructions[i][0] + " " + instructions[i][1], ha="left", va="top", color="k", size="larger")

ax.imshow(grid, cmap = cmap,norm=norm, interpolation='none')
plt.grid()
ax = plt.gca()
ax.set_xticks(np.arange(-.5, width, 1))
ax.set_yticks(np.arange(-.5, height, 1))
ax.set_xticklabels([])
ax.set_yticklabels([])
plt.show()