import os
from glob import glob
from setuptools import find_packages, setup


package_name = "vision_v1"

setup(
    name=package_name,
    version="1.0.0",
    packages=find_packages(exclude=["tests*"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        (os.path.join("share", package_name, "config"), glob("config/*.yaml")),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="THACO Drone",
    maintainer_email="minh.civil.en.2249@gmail.com",
    description="Decoupled RTSP video and FramePool YOLO pipeline for THACO Drone",
    license="Proprietary",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "vision_v1_node = vision_v1.vision_node:main",
        ],
    },
)
