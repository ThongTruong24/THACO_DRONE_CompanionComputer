import fs from 'fs';
import path from 'path';

const uaDir = '/home/lnh/Drone/.ua';
const meta = {
  lastAnalyzedAt: new Date().toISOString(),
  gitCommitHash: "f6efa576d395a6d1fb42cea3bda935f9cde98538",
  version: "1.0.0",
  analyzedFiles: 202
};

fs.writeFileSync(path.join(uaDir, 'meta.json'), JSON.stringify(meta, null, 2));
console.log('Saved meta.json');