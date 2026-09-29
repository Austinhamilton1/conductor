import csv
import sys
import pygame

# ============================================
# Configuration
# ============================================

CSV_FILE = 'engine_telemetry.csv'

WINDOW_WIDTH = 1200
WINDOW_HEIGHT = 800

BACKGROUND_COLOR = (20, 20, 20)
TARGET_COLOR = (180, 0, 220)

DRONE_RADIUS = 5
TARGET_RADIUS = 7

SIMULATION_DURATION = 10.0

# Extra space around the simulation bounds
WORLD_MARGIN = 50

# =============================================
# Data Types
# =============================================

class SimulationFrame:
    def __init__(
        self,
        time,
        entity_id,
        generation,
        position_x,
        position_y,
        velocity_x,
        velocity_y,
        target_x,
        target_y,
        battery_level,
        status,
    ):
        self.time = time
        self.entity_id = entity_id
        self.generation = generation

        self.position_x = position_x
        self.position_y = position_y

        self.velocity_x = velocity_x
        self.velocity_y = velocity_y

        self.target_x = target_x
        self.target_y = target_y

        self.battery_level = battery_level
        self.status = status

    @property
    def key(self):
        return (self.entity_id, self.generation)

# ===========================================
# CSV Loading
# ===========================================

def load_simulation(filename):
    frames = []

    with open(filename, 'r', newline='') as file:
        reader = csv.reader(file)

        for row_number, row in enumerate(reader, start=1):
            if not row:
                continue

            if row[0].strip().lower() in ('time', '{time'):
                continue

            if len(row) < 11:
                print(
                    f'Warning: skipping malformed row {row_number}: {row}',
                    file=sys.stderr
                )
                continue

            try:
                frame = SimulationFrame(
                    time=float(row[0]),
                    entity_id=int(row[1]),
                    generation=int(row[2]),
                    position_x=float(row[3]),
                    position_y=float(row[4]),
                    velocity_x=float(row[5]),
                    velocity_y=float(row[6]),
                    target_x=float(row[7]),
                    target_y=float(row[8]),
                    battery_level=float(row[9]),
                    status=row[10],
                )

                frames.append(frame)

            except ValueError as error:
                print(
                    f'Warning: skipping row {row_number}: {error}',
                    file=sys.stderr
                )

    frames.sort(key=lambda frame: frame.time)

    return frames

# ======================================
# Simulation Organization
# ======================================

def organize_frames(frames):
    timestamps = {}

    for frame in frames:
        if frame.time not in timestamps:
            timestamps[frame.time] = {}

        timestamps[frame.time][frame.key] = frame

    sorted_times = sorted(timestamps.keys())

    return sorted_times, timestamps

# ========================================
# Color
# ========================================

def battery_color(battery_level):
    battery = max(0.0, min(100.0, battery_level)) / 100.0

    red = int(255 * (1.0 - battery))
    green = int(255 * battery)

    return red, green, 0

# ===========================================
# Coordinate Conversion
# ===========================================

class WorldToScreen:
    def __init__(self, frames):
        positions = []

        for frame in frames:
            positions.append((frame.position_x, frame.position_y))
            positions.append((frame.target_x, frame.target_y))

        if not positions:
            self.min_x = 0.0
            self.max_x = 1.0
            self.min_y = 0.0
            self.max_y = 1.0
            return

        self.min_x = min(position[0] for position in positions)
        self.max_x = max(position[1] for position in positions)

        self.min_y = min(position[1] for position in positions)
        self.max_y = max(position[1] for position in positions)

        if self.min_x == self.max_x:
            self.max_x += 1.0

        if self.min_y == self.max_y:
            self.max_y += 1.0

    def convert(self, x, y):
        world_width = self.max_x - self.min_x
        world_height = self.max_y - self.min_y

        screen_width = WINDOW_WIDTH - WORLD_MARGIN * 2
        screen_height = WINDOW_HEIGHT - WORLD_MARGIN * 2

        scale_x = screen_width / world_width
        scale_y = screen_height / world_height

        scale = min(scale_x, scale_y)

        x_screen = (
            WORLD_MARGIN
            + (x - self.min_x) * scale
        )

        y_screen = (
            WINDOW_HEIGHT
            - WORLD_MARGIN
            - (y - self.min_y) * scale
        )

        return int(x_screen), int(y_screen)

# ================================
# State Reconstruction
# ================================

def get_state_at_time(sorted_times, timestamps, simulation_time):
    if not sorted_times:
        return {}

    current_time = sorted_times[0]

    for timestamp in sorted_times:
        if timestamp > simulation_time:
            break

        current_time = timestamp

    return timestamps[current_time]

# ==================================
# Rendering
# ==================================

def draw_simulation(
    screen,
    state,
    world_to_screen,
    simulation_time,
):
    screen.fill(BACKGROUND_COLOR)

    targets = set()

    for frame in state.values():
        targets.add((frame.target_x, frame.target_y))

    for target_x, target_y in targets:
        x, y = world_to_screen.convert(target_x, target_y)

        pygame.draw.circle(
            screen,
            TARGET_COLOR,
            (x, y),
            TARGET_RADIUS,
        )

    for frame in state.values():
        x, y = world_to_screen.convert(
            frame.position_x,
            frame.position_y,
        )

        color = battery_color(frame.battery_level)

        pygame.draw.circle(
            screen,
            color,
            (x, y),
            DRONE_RADIUS,
        )

    font = pygame.font.Font(None, 28)

    time_text = font.render(
        f'Simulation Time: {simulation_time:6.2f} s',
        True,
        (255, 255, 255),
    )

    screen.blit(
        time_text,
        (15, 15),
    )

    drone_text = font.render(
        f'Drones: {len(state)}',
        True,
        (255, 255, 255),
    )

    screen.blit(
        drone_text,
        (15, 45),
    )

    pygame.display.flip()

# =======================================
# Main
# =======================================

def main():
    pygame.init()

    screen = pygame.display.set_mode(
        (WINDOW_WIDTH, WINDOW_HEIGHT)
    )

    pygame.display.set_caption(
        "Conductor Simulation Visualizer"
    )

    clock = pygame.time.Clock()

    print(f'Loading simulation from {CSV_FILE}...')

    frames = load_simulation(CSV_FILE)

    if not frames:
        print('No simulation data found.')
        pygame.quit()
        return

    print(f'Loaded {len(frames)} records.')

    sorted_times, timestamps = organize_frames(frames)

    print(
        f'Simulation time: '
        f'{sorted_times[0]:.3f} -> {sorted_times[-1]:.3f}'
    )

    world_to_screen = WorldToScreen(frames)

    simulation_time = sorted_times[0]

    running = True
    paused = False

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    paused = not paused

                elif event.key == pygame.K_r:
                    simulation_time = sorted_times[0]
                    paused = False

                elif event.key == pygame.K_ESCAPE:
                    running = False

        if not paused:
            real_dt = clock.get_time() / 1000.0

            simulation_time += real_dt

            if simulation_time >= SIMULATION_DURATION:
                simulation_time = SIMULATION_DURATION
                paused = True

            state = get_state_at_time(
                sorted_times,
                timestamps,
                simulation_time,
            )

            draw_simulation(
                screen,
                state,
                world_to_screen,
                simulation_time,
            )

            clock.tick(60)

    pygame.quit()

if __name__ == '__main__':
    main()