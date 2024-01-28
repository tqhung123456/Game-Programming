import json
import random
import pygame
import math

# Initialize Pygame
pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=512)
pygame.init()

# Load the background image
background = pygame.image.load("images/bg.png")
screen_width, screen_height = background.get_size()

# Set the dimensions of the window
screen = pygame.display.set_mode((screen_width, screen_height))

# Load the large image containing the graves
graveyard_image = pygame.image.load("images/graves.png")

# Read positions from the JSON file
with open("positions.json", "r") as file:
    data = json.load(file)
    extraction_positions = [
        (item["x"], item["y"]) for item in data["extraction_positions"]
    ]
    placement_positions = [
        (item["x"], item["y"]) for item in data["placement_positions"]
    ]
    popup_positions = [(item["x"], item["y"]) for item in data["popup_positions"]]
    dying_positions = [(item["x"], item["y"]) for item in data["dying_positions"]]

# Extract each grave based on its position and size (74x84)
grave_size = (74, 84)
graves = [
    graveyard_image.subsurface(pygame.Rect(pos, grave_size))
    for pos in extraction_positions
]

# Blit the graves onto the background at the placement positions
for grave, pos in zip(graves, placement_positions):
    background.blit(grave, pos)

# Load and prepare zombie popping up images
zombie_popping_image = pygame.image.load("images/zombie.png")
(
    zombie_popping_image_width,
    zombie_popping_image_height,
) = zombie_popping_image.get_size()
popup_frames = []
frame_height = zombie_popping_image_height / 2  # Half the height of the original image
frame_rate = 30  # Adjust as needed for desired animation speed
for i in range(frame_rate + 1):
    frame = zombie_popping_image.subsurface(
        pygame.Rect(
            0, i * frame_height / frame_rate, zombie_popping_image_width, frame_height
        )
    )
    popup_frames.append(frame)

# Generate popup rectangles
popup_rects = []
for pos in popup_positions:
    popup_rects.append(pygame.Rect(pos, (zombie_popping_image_width, frame_height)))

# Load and prepare zombie dying images
zombie_dying_images = []
for i in range(1, 12):
    image = pygame.image.load(f"images/die{i}.png")
    zombie_dying_images.append(image)

# Game variables
clock = pygame.time.Clock()
popup_frame_count = len(popup_frames)
dying_frame_count = len(zombie_dying_images)
popup_duration = 0.5 * 1000  # miliseconds for popping up
stayup_duration = 1 * 1000  # miliseconds for staying up
dying_duration = 0.5 * 1000  # miliseconds for dying
font_obj = pygame.font.Font("./fonts/GROBOLD.ttf", 31)
score = 0
misses = 0

# Sound effects
mainTrack = pygame.mixer.music.load("sounds/themesong.wav")
pygame.mixer.music.play()
hurtSound = pygame.mixer.Sound("sounds/hurt.wav")

# Main game loop
running = True
animation_started = False  # Flag to indicate if animation has started
zombie_not_hit = True  # Flag to indicate if the zombie has not been hit
zombie_hit = False  # Flag to indicate if the zombie has been hit
start_time = pygame.time.get_ticks()
index = random.randint(0, len(placement_positions) - 1)
popup_position = popup_positions[index]
dying_position = dying_positions[index]
popup_rect = popup_rects[index]

while running:
    # Draw the background
    screen.blit(background, (0, 0))

    # Update the player's score
    current_score_string = "SCORE: " + str(score)
    score_text = font_obj.render(current_score_string, True, (255, 255, 255))
    score_text_pos = score_text.get_rect()
    score_text_pos.centerx = screen_width / 3 * 1
    score_text_pos.centery = 26
    screen.blit(score_text, score_text_pos)
    # Update the player's misses
    current_misses_string = "MISSES: " + str(misses)
    misses_text = font_obj.render(current_misses_string, True, (255, 255, 255))
    misses_text_pos = misses_text.get_rect()
    misses_text_pos.centerx = screen_width / 3 * 2
    misses_text_pos.centery = 26
    screen.blit(misses_text, misses_text_pos)

    # Calculate the time elapsed
    current_time = pygame.time.get_ticks() - start_time

    # Start animation if the zombie has not been hit
    if zombie_not_hit:
        # Check for mouse click
        for event in pygame.event.get():
            if event.type == pygame.MOUSEBUTTONDOWN:
                # Get the position of the mouse
                mouse_pos = event.pos

                # Check if mouse collides with the rectangle
                if popup_rect.collidepoint(mouse_pos):
                    zombie_hit = True
                else:
                    misses += 1
            elif event.type == pygame.QUIT:
                running = False

        # Zombie popping up
        if current_time <= popup_duration:
            popup_frame_index = math.floor(
                current_time / popup_duration * popup_frame_count
            )
            popup_frame_index = min(popup_frame_index, popup_frame_count - 1)
            popup_frame = popup_frames[popup_frame_index]

            # Draw
            screen.blit(popup_frame, popup_position)

            # Check for zombie hit
            if zombie_hit:
                zombie_not_hit = False
                zombie_hit = False
                score += 1
                hurtSound.play()
                start_time = pygame.time.get_ticks()

        # Zombie staying up
        elif current_time <= popup_duration + stayup_duration:
            popup_frame = popup_frames[-1]
            # Draw
            screen.blit(popup_frame, popup_position)

            # Check for zombie hit
            if zombie_hit:
                zombie_not_hit = False
                zombie_hit = False
                score += 1
                hurtSound.play()
                start_time = pygame.time.get_ticks()

        # Zombie popping down
        elif current_time <= popup_duration + stayup_duration + popup_duration:
            popup_frame_index = math.floor(
                (
                    (current_time - popup_duration - stayup_duration)
                    / popup_duration
                    * popup_frame_count
                )
                + 1
            )
            popup_frame_index = min(popup_frame_index, popup_frame_count - 1)
            popup_frame = popup_frames[-popup_frame_index]

            # Draw
            screen.blit(popup_frame, popup_position)

            # Check for zombie hit
            if zombie_hit:
                zombie_hit = False

        # Restart the animation
        else:
            popup_frame = popup_frames[0]

            # Draw
            screen.blit(popup_frame, popup_position)

            # Reset the flags
            start_time = pygame.time.get_ticks()
            index = random.randint(0, len(placement_positions) - 1)
            popup_position = popup_positions[index]
            dying_position = dying_positions[index]
            popup_rect = popup_rects[index]

    # Zombie has been hit
    else:
        # Zombie dying
        if current_time <= dying_duration:
            dying_frame_index = math.floor(
                current_time / dying_duration * (dying_frame_count + 1)
            )
            dying_frame_index = min(dying_frame_index, dying_frame_count - 1)
            dying_frame = zombie_dying_images[dying_frame_index]

            # Draw
            screen.blit(dying_frame, dying_position)

            # Check for last frame
            if dying_frame_index == dying_frame_count - 1:
                # Reset the flags
                zombie_not_hit = True
                start_time = pygame.time.get_ticks()
                index = random.randint(0, len(placement_positions) - 1)
                popup_position = popup_positions[index]
                dying_position = dying_positions[index]
                popup_rect = popup_rects[index]

        else:
            # Reset the flags
            zombie_not_hit = True
            start_time = pygame.time.get_ticks()
            index = random.randint(0, len(placement_positions) - 1)
            popup_position = popup_positions[index]
            dying_position = dying_positions[index]
            popup_rect = popup_rects[index]

    # Refresh the screen
    pygame.display.flip()
    clock.tick()

pygame.quit()
