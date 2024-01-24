import pygame
import time

# Initialize Pygame
pygame.init()

# Constants
IMAGE_PATH = "images/graves.png"  # Replace with your image path
BACKGROUND_IMAGE_PATH = "images/bg.png"  # Replace with your background image path
NUM_ROWS = 4
NUM_COLS = 5
OBJECT_NUMBER = 10  # Replace with your desired number (1-20)
object_position = (100, 100)  # Replace with the desired position for debugging

# Load the object image
full_image = pygame.image.load(IMAGE_PATH)
image_width, image_height = full_image.get_size()
object_width = image_width // NUM_COLS
object_height = image_height // NUM_ROWS

# Calculate the position of the object
col = (OBJECT_NUMBER - 1) % NUM_COLS
row = (OBJECT_NUMBER - 1) // NUM_COLS

# Extract the specific object's image
object_rect = pygame.Rect(
    col * object_width, row * object_height, object_width, object_height
)
object_image = full_image.subsurface(object_rect)

# Load the background image
background_image = pygame.image.load(BACKGROUND_IMAGE_PATH)
background_width, background_height = background_image.get_size()

# Create a window of the size of the background
screen = pygame.display.set_mode((background_width, background_height))
pygame.display.set_caption("Object Viewer")

# Main loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Display the background
    screen.blit(background_image, (0, 0))

    # Display the object at the specified position
    screen.blit(object_image, object_position)

    # Update the display
    pygame.display.flip()

    # Delay to slow down the animation
    time.sleep(0.2)

# Quit Pygame
pygame.quit()
