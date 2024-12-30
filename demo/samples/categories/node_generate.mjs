// run `node node_generate.mjs` from the categories folder to generate the list.mjs file
import { readdir } from 'fs/promises';
import { writeFile } from 'fs/promises';
import path from 'path';

async function generateList() {
    const result = {};
    const dirs = await readdir('.', { withFileTypes: true });
    for (const dir of dirs) {
        if (dir.isDirectory()) {
            const categoryName = dir.name.charAt(0).toUpperCase() + dir.name.slice(1);
            result[categoryName] = {};
            const files = await readdir(dir.name);
            for (const file of files) {
                if (file.endsWith('.mjs')) {
                    const baseName = path.basename(file, '.mjs');
                    const prettyName = baseName
                        .replace(/([A-Z])/g, ' $1')
                        .trim();
                    const capitalized = prettyName.charAt(0).toUpperCase() + prettyName.slice(1);
                    result[categoryName][capitalized] = `./categories/${dir.name}/${file}`;
                }
            }
        }
    }
    const fileContent = `export default ${JSON.stringify(result, null, 2)}`;
    await writeFile('list.mjs', fileContent);
}
generateList().catch(console.error);
