import fs from 'fs';

const scanRaw = JSON.parse(fs.readFileSync('/home/lnh/Drone/.ua/intermediate/scan-raw.json', 'utf8'));
const importOut = JSON.parse(fs.readFileSync('/home/lnh/Drone/.ua/intermediate/import-output.json', 'utf8'));

// Extract languages
const langSet = new Set();
scanRaw.files.forEach(f => {
  if (f.language && f.language !== 'unknown') {
    langSet.add(f.language);
  }
});
const languages = Array.from(langSet).sort();

const frameworks = ['ROS 2', 'MAVLink', 'Docker', 'Docker Compose', 'CMake', 'OpenCV', 'GStreamer'];

const scanResult = {
  name: 'Drone Edge Companion Computer',
  description: 'Edge Companion Computer system for Raspberry Pi 5 supporting dual-band networking, MAVLink telemetry/routing, RealSense/RTSP video streaming, YOLO computer vision, hardware management, and ROS 2 MAVROS integration.',
  languages: languages,
  frameworks: frameworks,
  files: scanRaw.files,
  totalFiles: scanRaw.files.length,
  filteredByIgnore: scanRaw.filteredByIgnore,
  estimatedComplexity: scanRaw.files.length > 500 ? 'very-large' : (scanRaw.files.length > 100 ? 'large' : 'medium'),
  importMap: importOut.importMap
};

fs.writeFileSync('/home/lnh/Drone/.ua/intermediate/scan-result.json', JSON.stringify(scanResult, null, 2));
console.log('Saved scan-result.json with', scanResult.totalFiles, 'files');