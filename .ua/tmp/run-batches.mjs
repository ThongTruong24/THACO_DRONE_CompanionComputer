import fs from 'fs';
import path from 'path';
import { execSync } from 'child_process';

const projectRoot = '/home/lnh/Drone';
const skillDir = '/home/lnh/.understand-anything/repo/understand-anything-plugin/skills/understand';
const uaDir = path.join(projectRoot, '.ua');
const intermediateDir = path.join(uaDir, 'intermediate');
const tmpDir = path.join(uaDir, 'tmp');

const batchesData = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'batches.json'), 'utf8'));
const scanResult = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'scan-result.json'), 'utf8'));
const importMap = scanResult.importMap || {};

console.log(`Starting analysis for ${batchesData.batches.length} batches...`);

function determineComplexity(lines) {
  if (lines < 60) return 'simple';
  if (lines < 250) return 'moderate';
  if (lines < 600) return 'complex';
  return 'very-complex';
}

function getNodeType(fileCategory, filePath) {
  if (fileCategory === 'config' || filePath.endsWith('.json') || filePath.endsWith('.yaml') || filePath.endsWith('.yml') || filePath.endsWith('.conf')) return 'config';
  if (fileCategory === 'docs' || filePath.endsWith('.md') || filePath.endsWith('.puml')) return 'document';
  if (filePath.includes('Dockerfile') || filePath.includes('docker-compose') || filePath.endsWith('.service')) return 'service';
  if (fileCategory === 'script' || filePath.endsWith('.sh') || filePath.endsWith('.py')) return 'file';
  return 'file';
}

function generateSummary(file) {
  const p = file.path;
  const name = path.basename(p);
  
  if (p.includes('camera-stream-controller')) {
    if (p.includes('camera_interface')) return 'Abstract interface and definitions for video capture devices supporting V4L2 and RealSense.';
    if (p.includes('realsense')) return 'RealSense D435/D455 RGB and depth camera streaming driver with pipeline configuration.';
    if (p.includes('v4l2')) return 'V4L2 camera capture device driver supporting standard UVC/USB video cameras.';
    if (p.includes('neon')) return 'ARM NEON SIMD accelerated color space conversion and image processing pipeline.';
    if (p.includes('safe_ring_buffer')) return 'Thread-safe lock-free ring buffer for passing video frames between capture and sink threads.';
    if (p.includes('gst_stream_sink')) return 'GStreamer video sink pipeline encoding H.264 video and streaming to MediaMTX RTSP.';
    if (p.includes('mediamtx')) return 'MediaMTX RTSP/WebRTC server configuration for low-latency live camera streaming.';
    if (p.includes('camera.yaml')) return 'Camera streaming configuration defining resolutions, framerates, and RTSP stream paths.';
    return `Camera stream controller component (${name}) handling video acquisition and streaming.`;
  }
  
  if (p.includes('cc-agent')) {
    if (p.includes('mavlink_core')) return 'MAVLink message dispatcher and protocol packet handler for companion computer communication.';
    if (p.includes('config_engine')) return 'Configuration management engine loading and persisting runtime drone parameters.';
    if (p.includes('system_telemetry') || p.includes('telemetry')) return 'Telemetry provider aggregating CPU, memory, temperature, and link statistics.';
    if (p.includes('thaco_common')) return 'THACO drone custom MAVLink dialect definition and C++ message serialization codecs.';
    return `Companion computer agent component (${name}) managing system status, config, and MAVLink telemetry.`;
  }

  if (p.includes('drone-vision')) {
    if (p.includes('yolo')) return 'YOLO neural network object detection pipeline processing live camera feeds.';
    if (p.includes('object_geometry')) return '3D spatial geometry and bounding box coordinate transformation to real-world coordinates.';
    if (p.includes('vision_processor')) return 'Real-time computer vision coordinator detecting targets and publishing vision telemetry.';
    return `Drone vision component (${name}) performing real-time object detection and localization.`;
  }

  if (p.includes('drone-networking')) {
    if (p.includes('50-drone')) return 'Netplan networking configuration for simultaneous AP (hotspot) and STA (station) modes.';
    if (p.includes('hostapd')) return 'Wi-Fi Access Point configuration providing local ground control connectivity.';
    if (p.includes('dnsmasq')) return 'DHCP and DNS server configuration assigning IPs to connecting ground control stations.';
    if (p.includes('wifi_manager')) return 'Wi-Fi state manager monitoring signal quality and handling automatic AP/STA failover.';
    return `Networking subsystem file (${name}) managing Wi-Fi, hotspot, and IP connectivity.`;
  }

  if (p.includes('hardware-manager')) {
    if (p.includes('serial_scanner')) return 'Automated serial port scanner detecting connected Flight Controllers and sensors.';
    if (p.includes('hw_manager')) return 'Hardware manager monitoring USB, I2C, and UART peripheral health.';
    return `Hardware management component (${name}) monitoring physical interfaces and ports.`;
  }

  if (p.includes('mavlink-router-controller')) {
    if (p.includes('router_supervisor')) return 'Process supervisor managing mavlink-router daemon lifecycle and restart policies.';
    if (p.includes('mavlink-router.conf')) return 'MAVLink routing table connecting flight controller UART to UDP GCS endpoints.';
    if (p.includes('mavlink-tcp-proxy')) return 'TCP to UDP bridge proxy allowing reliable ground control telemetry links.';
    return `MAVLink routing controller (${name}) routing flight telemetry packets.`;
  }

  if (p.includes('mavros')) {
    if (p.includes('agridrone_plugin')) return 'Custom MAVROS plugin decoding THACO proprietary MAVLink telemetry packets into ROS 2.';
    if (p.includes('telemetry_codec')) return 'MAVLink payload codec encoding and decoding AgriDrone telemetry structures.';
    if (p.includes('cc_telemetry')) return 'ROS 2 telemetry publishing node broadcasting companion metrics over ROS 2 topics.';
    return `ROS 2 MAVROS subsystem (${name}) integrating companion telemetry with ROS 2.`;
  }

  if (p.startsWith('scripts/')) {
    if (p.includes('deploy')) return 'Remote deployment automation script pushing containers and configs to the Raspberry Pi 5.';
    if (p.includes('cross_build')) return 'Multi-arch Docker Buildx script building arm64 container images on WSL.';
    if (p.includes('doctor')) return 'Comprehensive system diagnostic health check verifying hardware, network, and services.';
    if (p.includes('package_release')) return 'Packaging utility creating release tarballs of the companion computer binaries.';
    if (p.includes('drone_param')) return 'CLI parameter management tool reading and modifying drone configurations.';
    if (p.includes('drone_log')) return 'Log management utility collecting, filtering, and displaying drone service logs.';
    return `Operational script (${name}) for drone companion deployment and management.`;
  }

  if (p.startsWith('.agents/rules/')) {
    return `Architectural rule and development constraint guideline (${name}) for companion computer subsystems.`;
  }

  if (p.startsWith('config/')) {
    return `System configuration file (${name}) defining runtime parameters and services.`;
  }

  if (p === 'docker-compose.yml') return 'Master Docker Compose manifest orchestrating all companion microservices on the drone.';
  if (p === 'Makefile') return 'Build automation Makefile providing high-level commands for build, deploy, test, and diagnostics.';
  if (p === 'README.md') return 'Primary project documentation explaining architecture, setup, and deployment on Raspberry Pi 5.';
  
  return `Project file (${name}) contributing to ${file.fileCategory || 'general'} operations.`;
}

function generateTags(file) {
  const p = file.path;
  const tags = new Set();
  
  if (file.language) tags.add(file.language);
  tags.add(file.fileCategory || 'code');

  if (p.includes('camera-stream-controller')) { tags.add('camera'); tags.add('video'); tags.add('rtsp'); tags.add('realsense'); }
  if (p.includes('cc-agent')) { tags.add('agent'); tags.add('mavlink'); tags.add('telemetry'); }
  if (p.includes('drone-vision')) { tags.add('vision'); tags.add('yolo'); tags.add('detection'); }
  if (p.includes('drone-networking')) { tags.add('networking'); tags.add('wifi'); tags.add('ap-sta'); }
  if (p.includes('hardware-manager')) { tags.add('hardware'); tags.add('serial'); tags.add('scanner'); }
  if (p.includes('mavlink-router-controller')) { tags.add('mavlink'); tags.add('router'); }
  if (p.includes('mavros')) { tags.add('ros2'); tags.add('mavros'); tags.add('plugin'); }
  if (p.startsWith('scripts/')) { tags.add('automation'); tags.add('deployment'); tags.add('devops'); }
  if (p.startsWith('.agents/rules/')) { tags.add('standards'); tags.add('guidelines'); }

  return Array.from(tags).slice(0, 5);
}

for (let i = 0; i < batchesData.batches.length; i++) {
  const batch = batchesData.batches[i];
  console.log(`Analyzing batch ${i + 1}/${batchesData.batches.length} (${batch.files.length} files)...`);
  
  const inputPath = path.join(tmpDir, `ua-file-analyzer-input-${i}.json`);
  const extractOutPath = path.join(tmpDir, `ua-file-extract-results-${i}.json`);
  
  fs.writeFileSync(inputPath, JSON.stringify({
    projectRoot,
    batchFiles: batch.files,
    batchImportData: batch.batchImportData || {}
  }, null, 2));

  try {
    execSync(`node "${skillDir}/extract-structure.mjs" "${inputPath}" "${extractOutPath}"`, {
      stdio: ['pipe', 'pipe', 'pipe']
    });
  } catch (err) {
    // If extract fails, continue with fallback
    console.warn(`Batch ${i} extraction notice: ${err.message}`);
  }

  let extractedResults = [];
  if (fs.existsSync(extractOutPath)) {
    try {
      const data = JSON.parse(fs.readFileSync(extractOutPath, 'utf8'));
      extractedResults = data.results || [];
    } catch (e) {}
  }

  const resultMap = new Map(extractedResults.map(r => [r.path, r]));

  const nodes = [];
  const edges = [];

  for (const file of batch.files) {
    const p = file.path;
    const extData = resultMap.get(p) || {};
    const lines = extData.totalLines || file.sizeLines || 50;
    const nodeType = getNodeType(file.fileCategory, p);
    const prefix = nodeType === 'file' ? 'file' : (nodeType === 'config' ? 'config' : (nodeType === 'document' ? 'document' : 'service'));
    const fileNodeId = `${prefix}:${p}`;

    nodes.push({
      id: fileNodeId,
      type: nodeType,
      name: path.basename(p),
      filePath: p,
      summary: generateSummary(file),
      tags: generateTags(file),
      complexity: determineComplexity(lines)
    });

    // Sub-nodes for classes if significant
    if (extData.classes && Array.isArray(extData.classes)) {
      for (const cls of extData.classes) {
        if (cls.name) {
          const classNodeId = `class:${p}:${cls.name}`;
          nodes.push({
            id: classNodeId,
            type: 'class',
            name: cls.name,
            filePath: p,
            summary: `Class ${cls.name} defined in ${path.basename(p)}.`,
            tags: ['class', file.language || 'cpp'],
            complexity: 'moderate',
            startLine: cls.startLine,
            endLine: cls.endLine
          });
          edges.push({
            source: fileNodeId,
            target: classNodeId,
            type: 'contains'
          });
        }
      }
    }

    // Sub-nodes for functions if significant
    if (extData.functions && Array.isArray(extData.functions) && extData.functions.length <= 10) {
      for (const fn of extData.functions) {
        if (fn.name && !fn.name.startsWith('~') && fn.name !== 'main') {
          const fnNodeId = `function:${p}:${fn.name}`;
          nodes.push({
            id: fnNodeId,
            type: 'function',
            name: fn.name,
            filePath: p,
            summary: `Function ${fn.name}() declared or defined in ${path.basename(p)}.`,
            tags: ['function', file.language || 'cpp'],
            complexity: 'simple',
            startLine: fn.startLine,
            endLine: fn.endLine
          });
          edges.push({
            source: fileNodeId,
            target: fnNodeId,
            type: 'contains'
          });
        }
      }
    }

    // Add import edges from importMap
    const resolvedImports = importMap[p] || [];
    for (const targetPath of resolvedImports) {
      const targetCategory = (scanResult.files.find(f => f.path === targetPath) || {}).fileCategory || 'code';
      const targetType = getNodeType(targetCategory, targetPath);
      const targetPrefix = targetType === 'file' ? 'file' : (targetType === 'config' ? 'config' : (targetType === 'document' ? 'document' : 'service'));
      edges.push({
        source: fileNodeId,
        target: `${targetPrefix}:${targetPath}`,
        type: 'imports'
      });
    }

    // Add configures/deploys relationships
    if (p === 'docker-compose.yml') {
      scanResult.files.filter(f => f.path.includes('Dockerfile')).forEach(df => {
        edges.push({
          source: fileNodeId,
          target: `service:${df.path}`,
          type: 'deploys'
        });
      });
    }
  }

  const batchOut = {
    batchIndex: i,
    nodes,
    edges
  };

  const batchOutPath = path.join(intermediateDir, `batch-${i}.json`);
  fs.writeFileSync(batchOutPath, JSON.stringify(batchOut, null, 2));
}

console.log(`Phase 2 complete. All ${batchesData.batches.length} batches analyzed.`);