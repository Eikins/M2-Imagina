var renderer = null,
    scene = null,
    camera = null,
    cube = null;

InitThreeJS();

function InitThreeJS()
{
    var canvas = document.getElementById("glcanvas");

    renderer = new THREE.WebGLRenderer({
        canvas: canvas,
        antialias: true
    });

    renderer.setSize(canvas.width, canvas.height);
    document.body.appendChild(renderer.domElement);

    SetupScene(canvas);

}

function SetupScene(canvas)
{
    scene = new THREE.Scene();

    camera = new THREE.Camera(45, canvas.width / canvas.height, 1, 400);
    camera.position.z = 20;
    camera.lookAt(scene.position);
    scene.add(camera);

    var light = new THREE.DirectionalLight(0xFFFFFF, 1.5);
    light.position.set(0, 0, 10);
    scene.add(light);

    var material = new THREE.MeshBasicMaterial({
        color: 0x0000FF
    });

    var geometry = new THREE.CubeBufferGeometry(2, 2, 2);
    cube = new THREE.Mesh(geometry, material);

    cube.rotation.x = Math.PI / 5;
    cube.rotation.y = Math.PI / 5;

    scene.add(cube);

    renderer.setAnimationLoop( () => {
        update();
        render();
    });
}

function update()
{

}

function render()
{
    renderer.render(scene, camera);
}