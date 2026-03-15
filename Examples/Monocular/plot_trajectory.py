#!/usr/bin/env python3
import sys
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def main():
    if len(sys.argv) < 2:
        print("Usage: python plot_trajectory.py <KeyFrameTrajectory.txt>")
        sys.exit(1)

    filename = sys.argv[1]
    
    try:
        # Columns: timestamp x y z qx qy qz qw
        data = np.loadtxt(filename)
    except Exception as e:
        print(f"Error reading file {filename}: {e}")
        sys.exit(1)

    if data.ndim == 1:
        data = data.reshape(1, -1)
        
    if data.shape[1] < 4:
        print("Error: The file does not contain enough columns for X, Y, Z.")
        sys.exit(1)

    x = data[:, 1]
    y = data[:, 2]
    z = data[:, 3]

    fig = plt.figure(figsize=(10, 8))
    
    # 3D Plot
    ax3d = fig.add_subplot(211, projection='3d')
    ax3d.plot(x, y, z, label='Estimated Trajectory', color='b', linewidth=1.5)
    ax3d.scatter(x[0], y[0], z[0], color='g', marker='o', s=50, label='Start')
    ax3d.scatter(x[-1], y[-1], z[-1], color='r', marker='x', s=50, label='End')
    ax3d.set_xlabel('X (m)')
    ax3d.set_ylabel('Y (m)')
    ax3d.set_zlabel('Z (m)')
    ax3d.set_title('3D Map Trajectory')
    ax3d.legend()

    # 2D Top-down (birds-eye) Plot
    ax2d = fig.add_subplot(212)
    ax2d.plot(x, z, label='Estimated Trajectory (XZ)', color='purple', linewidth=1.5)
    ax2d.scatter(x[0], z[0], color='g', marker='o', s=50, label='Start')
    ax2d.scatter(x[-1], z[-1], color='r', marker='x', s=50, label='End')
    ax2d.set_xlabel('X (m)')
    ax2d.set_ylabel('Z (m)') # Forward depth is usually Z in ORB SLAM camera frame
    ax2d.set_title('Bird\'s Eye View (XZ Plane)')
    ax2d.legend()
    ax2d.grid(True)
    ax2d.axis('equal')

    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
