import fs from 'fs';
const raw = JSON.parse(fs.readFileSync('/home/lnh/Drone/.ua/intermediate/scan-raw.json', 'utf8'));
const input = {
  projectRoot: '/home/lnh/Drone',
  files: raw.files
};
fs.writeFileSync('/home/lnh/Drone/.ua/intermediate/import-input.json', JSON.stringify(input, null, 2));
console.log('Wrote import-input.json with', input.files.length, 'files');