import fs from 'fs';
import path from 'path';

const graphPath = '/home/lnh/Drone/.ua/intermediate/assembled-graph.json';
const graph = JSON.parse(fs.readFileSync(graphPath, 'utf8'));

const fileLevelTypes = new Set(['file', 'config', 'document', 'service', 'pipeline', 'table', 'schema', 'resource', 'endpoint']);
const fileNodes = graph.nodes.filter(n => fileLevelTypes.has(n.type));

const layers = [
  {
    id: 'layer:orchestration-and-devops',
    name: 'Orchestration and DevOps',
    description: 'Docker orchestration, build scripts, deployment tools, developer rules, and environment configurations.',
    nodeIds: []
  },
  {
    id: 'layer:hardware-and-system-management',
    name: 'Hardware and Peripheral Management',
    description: 'Hardware discovery, flight controller serial scanning, system metrics collection, and hardware watchdog services.',
    nodeIds: []
  },
  {
    id: 'layer:telemetry-and-cc-agent',
    name: 'Companion Agent and Custom MAVLink',
    description: 'Central CC agent managing parameter storage, health states, and THACO proprietary MAVLink dialects.',
    nodeIds: []
  },
  {
    id: 'layer:mavlink-routing-and-proxy',
    name: 'MAVLink Routing Subsystem',
    description: 'Telemetry packet routing between autopilot UART, internal nodes, and external Ground Control Stations.',
    nodeIds: []
  },
  {
    id: 'layer:camera-and-video-streaming',
    name: 'Camera Acquisition and RTSP Streaming',
    description: 'V4L2/RealSense video capture, NEON color conversion, GStreamer pipeline, and MediaMTX RTSP broadcast.',
    nodeIds: []
  },
  {
    id: 'layer:computer-vision-detection',
    name: 'Computer Vision and Target Localization',
    description: 'Real-time YOLO object detection, 3D spatial geometry transformations, and tracking telemetry.',
    nodeIds: []
  },
  {
    id: 'layer:networking-and-connectivity',
    name: 'Network and AP-STA Connectivity',
    description: 'Dual-band Wi-Fi management, hotspot AP configuration, DHCP server, and network auto-failover.',
    nodeIds: []
  },
  {
    id: 'layer:ros2-and-mavros-integration',
    name: 'ROS 2 MAVROS Integration',
    description: 'ROS 2 Jazzy workspace and custom MAVROS plugins translating drone telemetry into ROS topics.',
    nodeIds: []
  }
];

const layerMap = {
  'orchestration-and-devops': layers[0],
  'hardware-and-system-management': layers[1],
  'telemetry-and-cc-agent': layers[2],
  'mavlink-routing-and-proxy': layers[3],
  'camera-and-video-streaming': layers[4],
  'computer-vision-detection': layers[5],
  'networking-and-connectivity': layers[6],
  'ros2-and-mavros-integration': layers[7]
};

fileNodes.forEach(node => {
  const p = node.filePath || '';
  if (p.includes('camera-stream-controller')) {
    layerMap['camera-and-video-streaming'].nodeIds.push(node.id);
  } else if (p.includes('cc-agent')) {
    layerMap['telemetry-and-cc-agent'].nodeIds.push(node.id);
  } else if (p.includes('drone-vision')) {
    layerMap['computer-vision-detection'].nodeIds.push(node.id);
  } else if (p.includes('drone-networking')) {
    layerMap['networking-and-connectivity'].nodeIds.push(node.id);
  } else if (p.includes('mavlink-router-controller')) {
    layerMap['mavlink-routing-and-proxy'].nodeIds.push(node.id);
  } else if (p.includes('hardware-manager')) {
    layerMap['hardware-and-system-management'].nodeIds.push(node.id);
  } else if (p.includes('mavros')) {
    layerMap['ros2-and-mavros-integration'].nodeIds.push(node.id);
  } else if (p.startsWith('scripts/') || p.startsWith('config/')) {
    layerMap['hardware-and-system-management'].nodeIds.push(node.id);
  } else {
    layerMap['orchestration-and-devops'].nodeIds.push(node.id);
  }
});

fs.writeFileSync('/home/lnh/Drone/.ua/intermediate/layers.json', JSON.stringify(layers, null, 2));
console.log('Saved layers.json with', layers.length, 'layers.');
layers.forEach(l => console.log(`- ${l.name}: ${l.nodeIds.length} nodes`));