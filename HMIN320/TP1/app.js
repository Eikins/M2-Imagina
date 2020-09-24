var renderer = null,
    scene = null,
    camera = null,
    carousel = null,
    cup = null,
    cup2 = null;

initThreeJS(document.getElementById("webgl-container"));

function onContainerResize() {
    let parent = renderer.domElement.parentElement;
    let width = parent.clientWidth;
    let height = parent.clientHeight;

    camera.aspect = width / height;
    camera.updateProjectionMatrix();
    renderer.setSize(width, height);
}

function initThreeJS(containerElement)
{
    renderer = new THREE.WebGLRenderer({
        antialias: true
    });
    renderer.physicallyCorrectLights = true;
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    renderer.setSize(containerElement.clientWidth, containerElement.clientHeight);

    // Attach a resize observer
    // This method works better than using the window, and help for
    // situations where the canvas is not full screen
    new ResizeObserver(onContainerResize).observe(containerElement);
    containerElement.appendChild(renderer.domElement);
    setupScene(containerElement);
    populateScene();
}

function setupScene(containerElement)
{
    scene = new THREE.Scene();

    // CAMERA
    camera = new THREE.PerspectiveCamera(45, containerElement.clientWidth / containerElement.clientHeight, 1, 400);
    camera.position.z = 20;
    camera.lookAt(0, 0, 0);

    // CONTROLS & UTILS
    let orbitControls = new THREE.OrbitControls(camera);
    let axesHelper = new THREE.AxesHelper(5);

    // LIGHTS
    let light = new THREE.DirectionalLight(0xFFFFFF, 1.5);
    light.position.set(0, 30, 30);
    light.lookAt(0, 0, 0);
    light.castShadow = true;
    light.shadowCameraVisible = true;
    light.shadow.camera.left = -20;
    light.shadow.camera.right = 20;
    light.shadow.camera.top = 20;
    light.shadow.camera.bottom = -20;
    let ambientLight = new THREE.HemisphereLight(0xDDEEFF, 0x202020, 3);

    // SKYBOX
    scene.background = new THREE.CubeTextureLoader().load([
        "textures/cube/posx.jpg",
        "textures/cube/negx.jpg",
        "textures/cube/posy.jpg",
        "textures/cube/negy.jpg",
        "textures/cube/posz.jpg",
        "textures/cube/negz.jpg",
    ]);
    

    scene.add(camera);
    scene.add(axesHelper);
    scene.add(light);
    scene.add(ambientLight);

    renderer.setAnimationLoop( () => {
        update();
        render();
    });
}

function populateScene()
{
    // CAROUSEL
    let groundMaterial = new THREE.MeshPhongMaterial({
        side: THREE.FrontSide,
        map: new THREE.TextureLoader().load("./textures/spiral-background2.jpeg"),
        shininess: 2
    });
    let groundGeometry = new THREE.CylinderBufferGeometry(15, 15, 1, 32);

    let columnMaterial = new THREE.MeshStandardMaterial({
        side: THREE.FrontSide,
        map: new THREE.TextureLoader().load("./textures/gold.jpg"),
        roughness: 0.3,
        metalness: 1
    });
    let columnGeometry = new THREE.CylinderBufferGeometry(1, 1, 10, 16);

    let hatMaterial = new THREE.MeshPhongMaterial({
        side: THREE.FrontSide,
        map: new THREE.TextureLoader().load("./textures/spiral-background.jpg"),
        shininess: 2
    });
    hatMaterial.map.wrapS = THREE.RepeatMirror;
    hatMaterial.map.wrapT = THREE.RepeatMirror;
    hatMaterial.map.repeat.set(10, 1);
    let hatGeometry = new THREE.CylinderBufferGeometry(1, 15, 3, 32);

    ground = new THREE.Mesh(groundGeometry, groundMaterial);
    ground.receiveShadow = true;
    column = new THREE.Mesh(columnGeometry, columnMaterial);
    column.position.y += 5;
    column.castShadow = true;
    hat = new THREE.Mesh(hatGeometry, hatMaterial);
    hat.position.y += 10
    hat.castShadow = true;

    carousel = new THREE.Group();
    carousel.add(ground);
    carousel.add(column);
    carousel.add(hat);
    scene.add(carousel);

    let cupLoader = new THREE.MTLLoader().setResourcePath("./models/Cups/").setPath("./models/Cups/");
    cupLoader.load("cup2.mtl", (materials) => {
        materials.preload();
        new THREE.OBJLoader().setMaterials(materials).setPath("./models/Cups/").load("cup.obj", (object) => {
            object.scale.set(0.1, 0.1, 0.1);
            object.position.set(10, 2, 0);
            carousel.add(object);
            cup2 = object;
            object.traverse((node) => {
                if ( node instanceof THREE.Mesh ) 
                {
                    node.castShadow = true;
                    node.receiveShadow = true;
                } 
            });
        })
    });
    cupLoader.load("cup.mtl", (materials) => {
        materials.preload();
        new THREE.OBJLoader().setMaterials(materials).setPath("./models/Cups/").load("cup.obj", (object) => {
            object.scale.set(0.1, 0.1, 0.1);
            object.position.set(-10, 2, 0);
            carousel.add(object);
            cup = object;
            object.traverse((node) => {
                if ( node instanceof THREE.Mesh ) 
                {
                    node.castShadow = true;
                    node.receiveShadow = true;
                } 
            });
        })
    });

    new THREE.MTLLoader().setResourcePath("./models/R2D2/").setPath("./models/R2D2/").load("r2-d2.mtl", (materials) => {
        materials.preload();
        new THREE.OBJLoader().setMaterials(materials).setPath("./models/R2D2/").load("r2-d2.obj", (object) => {
            object.scale.set(0.3, 0.3, 0.3);
            object.position.set(0, 0, 0);
            cup2.add(object);
            object.traverse((node) => {
                if ( node instanceof THREE.Mesh ) 
                {
                    node.castShadow = true;
                    node.receiveShadow = true;
                } 
            });
        })
    });
}

function update()
{
    carousel.rotation.y += 0.005;
    if (cup) cup.rotation.y += 0.01;
    if (cup2) cup2.rotation.y += 0.04
}

function render()
{
    renderer.render(scene, camera);
}