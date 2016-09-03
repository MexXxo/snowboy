Install dependencies:
``` bash
npm install -g nan node-pre-gyp
./install_portaudio.sh
```

Configure and build with `node-pre-gyp`
``` bash
node-pre-gyp clean configure build
```

Run the example
```
node example.js
```