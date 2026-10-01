import fs from 'fs';
import path from 'path';

const projectRoot = '/home/lnh/Drone';
const intermediateDir = path.join(projectRoot, '.ua/intermediate');
const uaDir = path.join(projectRoot, '.ua');

// 1. Write knowledge-graph.json
fs.copyFileSync(path.join(intermediateDir, 'assembled-graph.json'), path.join(uaDir, 'knowledge-graph.json'));
console.log('Saved knowledge-graph.json');

// 2. Prepare fingerprint-input.json
const scanResult = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'scan-result.json'), 'utf8'));
const filePaths = scanResult.files.map(f => f.path);

const fpInput = {
  projectRoot,
  filePaths,
  gitCommitHash: "f6efa576d395a6d1fb42cea3bda935f9cde98538"
};

fs.writeFileSync(path.join(intermediateDir, 'fingerprint-input.json'), JSON.stringify(fpInput, null, 2));
console.log('Wrote fingerprint-input.json with', filePaths.length, 'files');