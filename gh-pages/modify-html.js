const fs = require('fs');
const path = require('path');

function findHtmlFiles(dir) {
	let results = [];
	const files = fs.readdirSync(dir);

	for (const file of files) {
		const fullPath = path.join(dir, file);
		const stat = fs.statSync(fullPath);

		if (fullPath.includes('node_modules')) {
			continue;
		}

		if (stat.isDirectory()) {
			results = results.concat(findHtmlFiles(fullPath));
		} else if (path.extname(file).toLowerCase() === '.html') {
			results.push(fullPath);
		}
	}

	return results;
}

function insertScript(htmlContent) {
	if (htmlContent.includes('coi-serviceworker.min.js')) {
		return htmlContent;
	}
	return htmlContent.replace(
		/<head>/i,
		'<head>\n  <script src="/box2d3-wasm/coi-serviceworker.min.js"></script>'
	);
}

const htmlFiles = findHtmlFiles('./');

for (const file of htmlFiles) {
	console.log(`Processing ${file}...`);
	const content = fs.readFileSync(file, 'utf8');
	const modified = insertScript(content);
	fs.writeFileSync(file, modified);
}
