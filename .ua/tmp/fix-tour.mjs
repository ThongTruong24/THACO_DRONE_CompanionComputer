import fs from 'fs';
const graphPath = '/home/lnh/Drone/.ua/intermediate/assembled-graph.json';
const graph = JSON.parse(fs.readFileSync(graphPath, 'utf8'));

// Fix tour step node reference
graph.tour.forEach(step => {
  step.nodeIds = step.nodeIds.map(id => id === 'service:docker-compose.yml' ? 'config:docker-compose.yml' : id);
});

fs.writeFileSync(graphPath, JSON.stringify(graph, null, 2));
console.log('Fixed tour step reference in assembled-graph.json');