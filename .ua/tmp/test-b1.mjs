import fs from 'fs';
const batches = JSON.parse(fs.readFileSync('/home/lnh/Drone/.ua/intermediate/batches.json', 'utf8'));
const b1 = batches.batches[1];
const input = {
  projectRoot: '/home/lnh/Drone',
  batchFiles: b1.files,
  batchImportData: b1.batchImportData || {}
};
fs.writeFileSync('/home/lnh/Drone/.ua/tmp/ua-file-analyzer-input-1.json', JSON.stringify(input, null, 2));
console.log('Prepared batch 1 input with', input.batchFiles.length, 'files');