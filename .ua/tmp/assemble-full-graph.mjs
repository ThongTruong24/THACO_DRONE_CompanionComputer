import fs from 'fs';
import path from 'path';

const projectRoot = '/home/lnh/Drone';
const intermediateDir = path.join(projectRoot, '.ua/intermediate');

const scanResult = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'scan-result.json'), 'utf8'));
const assembledGraph = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'assembled-graph.json'), 'utf8'));
const layers = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'layers.json'), 'utf8'));
const tour = JSON.parse(fs.readFileSync(path.join(intermediateDir, 'tour.json'), 'utf8'));

// Assemble full knowledge graph
const fullGraph = {
  name: scanResult.name,
  description: scanResult.description,
  languages: scanResult.languages,
  frameworks: scanResult.frameworks,
  nodes: assembledGraph.nodes,
  edges: assembledGraph.edges,
  layers: layers,
  tour: tour
};

const fullGraphPath = path.join(intermediateDir, 'assembled-graph.json');
fs.writeFileSync(fullGraphPath, JSON.stringify(fullGraph, null, 2));
console.log('Assembled full knowledge graph to assembled-graph.json');
console.log(`Summary: ${fullGraph.nodes.length} nodes, ${fullGraph.edges.length} edges, ${fullGraph.layers.length} layers, ${fullGraph.tour.length} tour steps.`);