from setuptools import find_packages, setup

package_name = "networking"

setup(
    name=package_name,
    version="1.0.0",
    packages=find_packages(exclude=["tests"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="THACO Drone",
    maintainer_email="minh.civil.en.2249@gmail.com",
    description="Drone Networking Controller and ROS 2 Network Monitor for THACO Drone",
    license="Proprietary",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "networking_node = networking.networking_node:main",
        ],
    },
)
