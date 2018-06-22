var gl;

function clamp(num, min, max) {
  return num <= min ? min : num >= max ? max : num;
}

function degToRad(degrees) {
  return degrees * Math.PI / 180;
}


var deltaTime = 0.0;

//Load level as binary resource
var map_data = load_binary_resource("ar_baggage_raw.tbsp");
var level = new tbsp_level(map_data);

//var model_data = load_binary_resource("test.tmdl");
//var model_test = new tbsp_model(model_data);

function initGL(canvas) {
    try {
        gl = canvas.getContext("experimental-webgl");
        gl.viewportWidth = canvas.width;
        gl.viewportHeight = canvas.height;
    } catch (e) {
    }
    if (!gl) {
        alert("Could not initialise WebGL, sorry :-(");
    }
}


function camera(){
  this.front = vec3.create([0.0, 0.0, 1.0]);
  this.up = vec3.create([0.0, 1.0, 0.0]);
  this.pos = vec3.create([-1.5, 6.8, -11]);
  this.viewMatrix = mat4.create();

  this.lastx = 0;
  this.lasty = 0;

  this.yaw = 0;
  this.pitch = 0;
  this.roll = 0;

  this.sensitivity = 0.1;

  this.moveForward = function(amount)
  {
    //this.pos += amount * this.front;
    var temp = vec3.create();
    vec3.add(temp, this.front);
    vec3.multiply(temp, [amount * deltaTime, amount * deltaTime, amount * deltaTime]);

    vec3.add(this.pos, temp);
  }

  this.moveHorizontal = function(amount)
  {
    var travelVector = vec3.create();
    vec3.cross(this.front, this.up, travelVector);
    vec3.normalize(travelVector);
    vec3.multiply(travelVector, [amount * deltaTime, amount * deltaTime, amount * deltaTime])

    vec3.add(this.pos, travelVector);
  }

  this.getMatrix = function()
  {
    mat4.identity(this.viewMatrix); //Reset view matrix

    var temp = vec3.create();
    vec3.add(temp, this.pos);
    vec3.add(temp, this.front);

    mat4.lookAt(this.pos, temp, this.up, this.viewMatrix);
    return this.viewMatrix; //Return it
  }

  this.mouseUpdate = function(xpos, ypos)
  {
    var xoffset = xpos;// - this.lastx;
    var yoffset = ypos;// - this.lasty;

    this.lastx = xpos;
    this.lasty = ypos;

    xoffset *= -this.sensitivity;
    yoffset *= -this.sensitivity;

    //this.yaw = this.yaw % 360.0;
    this.pitch = clamp(this.pitch + yoffset, -89, 89);
    this.yaw += xoffset;
    //this.pitch += yoffset;

    //Front facing vector
    var front = vec3.create();
    front[2] = Math.cos(degToRad(this.pitch)) * Math.cos(degToRad(this.yaw));
    front[1] = Math.sin(degToRad(this.pitch));
    front[0] = Math.cos(degToRad(this.pitch)) * Math.sin(degToRad(this.yaw));

    //Update class vectors
    this.front = front;
  }
}

//test camera
var test_camera = new camera();

var tMatrix = test_camera.getMatrix();

var shaderProgram;


function getShader(gl, id) {
    var shaderScript = document.getElementById(id);
    if (!shaderScript) {
        return null;
    }

    var str = "";
    var k = shaderScript.firstChild;
    while (k) {
        if (k.nodeType == 3) {
            str += k.textContent;
        }
        k = k.nextSibling;
    }

    var shader;
    if (shaderScript.type == "x-shader/x-fragment") {
        shader = gl.createShader(gl.FRAGMENT_SHADER);
    } else if (shaderScript.type == "x-shader/x-vertex") {
        shader = gl.createShader(gl.VERTEX_SHADER);
    } else {
        return null;
    }

    gl.shaderSource(shader, str);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        alert(gl.getShaderInfoLog(shader));
        return null;
    }

    return shader;
}
function shader(name)
{
  //Get shader sources from html
  var fragShaderSource = getShader(gl, name + "-fs");
  var vertShaderSource = getShader(gl, name + "-vs");

  //Create program
  this.program = gl.createProgram();
  gl.attachShader(this.program, vertShaderSource);
  gl.attachShader(this.program, fragShaderSource);
  //Link shaders
  gl.linkProgram(this.program);

  //Check for errors
  if (!gl.getProgramParameter(this.program, gl.LINK_STATUS)) {
      alert("SHADER::INITIALIZE::FAIL SHADER(" + name + ")");
  }

  gl.enableVertexAttribArray(gl.getAttribLocation(this.program, "aVertexPosition"));
  gl.enableVertexAttribArray(gl.getAttribLocation(this.program, "aVertexNormal"));
  gl.enableVertexAttribArray(gl.getAttribLocation(this.program, "aTextureCoord"));

  //==============================================================================
  // SHADER PARAMETER FUNCTIONS
  this.setMatrix4fv = function(name, matrix){
    gl.uniformMatrix4fv(gl.getUniformLocation(this.program, name), false, matrix);
  }

  this.setMatrix3fv = function(name, matrix){
    gl.uniformMatrix3fv(gl.getUniformLocation(this.program, name), false, matrix);
  }

  this.setVector3f = function(name, v0, v1, v2){
    gl.uniform3f(gl.getUniformLocation(this.program, name), v0, v1, v2);
  }

  this.setVector3fv = function(name, vector){
    gl.uniform3fv(gl.getUniformLocation(this.program, name), vector);
  }

  this.setFloat = function(name, value){
    gl.uniform1f(gl.getUniformLocation(this.program, name), value);
  }

  this.setInt = function(name, value){
    gl.uniform1i(gl.getUniformLocation(this.program, name), value);
  }

  /* Activate this shader */
  this.use = function(){
    gl.useProgram(this.program);
  }
}

var testShader;


function initShaders() {
  testShader = new shader("shader");
  return;
  /*
  var fragmentShader = getShader(gl, "shader-fs");
  var vertexShader = getShader(gl, "shader-vs");

  shaderProgram = gl.createProgram();
  gl.attachShader(shaderProgram, vertexShader);
  gl.attachShader(shaderProgram, fragmentShader);
  gl.linkProgram(shaderProgram);

  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
      alert("Could not initialise shaders");
  }

  gl.useProgram(shaderProgram);

  shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
  gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

  shaderProgram.vertexNormalAttribute = gl.getAttribLocation(shaderProgram, "aVertexNormal");
  gl.enableVertexAttribArray(shaderProgram.vertexNormalAttribute);

  shaderProgram.textureCoordAttribute = gl.getAttribLocation(shaderProgram, "aTextureCoord");
  gl.enableVertexAttribArray(shaderProgram.textureCoordAttribute);

  shaderProgram.pMatrixUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
  shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
  shaderProgram.nMatrixUniform = gl.getUniformLocation(shaderProgram, "uNMatrix");
  shaderProgram.samplerUniform = gl.getUniformLocation(shaderProgram, "uSampler");
  shaderProgram.viewMatrixUniform = gl.getUniformLocation(shaderProgram, "viewMatrix")
  shaderProgram.useLightingUniform = gl.getUniformLocation(shaderProgram, "uUseLighting");
  shaderProgram.ambientColorUniform = gl.getUniformLocation(shaderProgram, "uAmbientColor");
  shaderProgram.lightingDirectionUniform = gl.getUniformLocation(shaderProgram, "uLightingDirection");
  shaderProgram.directionalColorUniform = gl.getUniformLocation(shaderProgram, "uDirectionalColor");*/
}

function handleLoadedTexture(texture) {
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture.image);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
    gl.generateMipmap(gl.TEXTURE_2D);

    gl.bindTexture(gl.TEXTURE_2D, null);
}

var crateTexture;
var modelTexture;

function initTexture() {
    crateTexture = gl.createTexture();
    crateTexture.image = new Image();
    crateTexture.image.onload = function () {
        handleLoadedTexture(crateTexture);
    }

    crateTexture.image.src = "bsp.png";


    modelTexture = gl.createTexture();
    modelTexture.image = new Image();
    modelTexture.image.onload = function (){
      handleLoadedTexture(modelTexture);
    }

    modelTexture.image.src = "model.png";
}


var mvMatrix = mat4.create();
var viewMatrix = mat4.create();
var mvMatrixStack = [];
var pMatrix = mat4.create();

function mvPushMatrix() {
    var copy = mat4.create();
    mat4.set(mvMatrix, copy);
    mvMatrixStack.push(copy);
}

function mvPopMatrix() {
  if (mvMatrixStack.length == 0) {
    throw "Invalid popMatrix!";
  }
  mvMatrix = mvMatrixStack.pop();
}


function setMatrixUniforms() {
  return;
  gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
  gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
  gl.uniformMatrix4fv(shaderProgram.viewMatrixUniform, false, test_camera.getMatrix());

  var normalMatrix = mat3.create();
  mat4.toInverseMat3(mvMatrix, normalMatrix);
  mat3.transpose(normalMatrix);
  gl.uniformMatrix3fv(shaderProgram.nMatrixUniform, false, normalMatrix);
}







var xRot = 0;
var xSpeed = 3;

var yRot = 0;
var ySpeed = -3;

var z = -5.0;

var currentlyPressedKeys = {};
function handleKeyDown(event) {
    currentlyPressedKeys[event.keyCode] = true;
}
function handleKeyUp(event) {
    currentlyPressedKeys[event.keyCode] = false;
}

var mouse_x = 0;
var mouse_y = 0;

var clicking = false;
function handleKeys() {
    if (currentlyPressedKeys[33]) {
        // Page Up
        z -= 0.05;
    }
    if (currentlyPressedKeys[34]) {
        // Page Down
        z += 0.05;
    }
    if (currentlyPressedKeys[37]) {
        // Left cursor key
        ySpeed -= 1;
    }
    if (currentlyPressedKeys[39]) {
        // Right cursor key
        ySpeed += 1;
    }
    if (currentlyPressedKeys[38]) {
        // Up cursor key
        xSpeed -= 1;
    }
    if (currentlyPressedKeys[40]) {
        // Down cursor key
        xSpeed += 1;
    }

    //Camera controller
    // W key
    if (currentlyPressedKeys[87]){
      test_camera.moveForward(0.5);
    }
    //A key
    if(currentlyPressedKeys[65]){
      test_camera.moveHorizontal(-0.5);
    }
    //S key
    if(currentlyPressedKeys[83]){
      test_camera.moveForward(-0.5);
    }
    //D key
    if(currentlyPressedKeys[68]){
      test_camera.moveHorizontal(0.5);
    }
}
function handleMouseMove(event){
    //test_camera.mouseUpdate(event.clientX, event.clientY);
  if(event.which ==1)
    test_camera.mouseUpdate(event.movementX, event.movementY);
}
function handleClick(event){
  clicking = true;
}
function handleClickUp(event){
  clicking = false;
}

function gl_mesh(verts, normals, uvs, indices){
  //=======================================================================
  // Create vertex position buffer
  this.vertexPositionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexPositionBuffer);
  //Upload vertex positions to GPU
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(verts), gl.STATIC_DRAW);
  this.vertexPositionBuffer.itemSize = 3; //VEC3
  this.vertexPositionBuffer.numItems = verts.length / 3;

  //=======================================================================
  // Create normal vector buffer
  this.normalBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, this.normalBuffer);
  //Upload normal vectors
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(normals), gl.STATIC_DRAW);
  this.normalBuffer.itemSize = 3;
  this.normalBuffer.numItems = normals.length / 3;

  //======================================================================
  // Create Texture coordinaate buffers
  this.texCoordBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
  //Upload texture textureCoordinates
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(uvs), gl.STATIC_DRAW);
  this.texCoordBuffer.itemSize = 2;
  this.texCoordBuffer.numItems = uvs.length / 2;

  //=====================================================================
  // Create Indices buffers
  this.indicesBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indicesBuffer);
  //Upload indices
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
  this.indicesBuffer.itemSize = 1;
  this.indicesBuffer.numItems = indices.length;

  //=====================================================================
  // Setup buffer pointers
  /*
  gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexPositionBuffer);
  gl.vertexAttribPointer(0, this.vertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

  gl.bindBuffer(gl.ARRAY_BUFFER, this.normalBuffer);
  gl.vertexAttribPointer(1, this.normalBuffer.itemSize, gl.FLOAT, false, 0, 0);

  gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
  gl.vertexAttribPointer(2, this.texCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);*/

  console.log(this.vertexPositionBuffer);
  console.log(this.normalBuffer);

  this.draw = function()
  {
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexPositionBuffer);
    gl.vertexAttribPointer(0, this.vertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.normalBuffer);
    gl.vertexAttribPointer(1, this.normalBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
    gl.vertexAttribPointer(2, this.texCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);


    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indicesBuffer);

    //Draw
    gl.drawElements(gl.TRIANGLES, this.indicesBuffer.numItems, gl.UNSIGNED_SHORT, 0);
  }
}

//Create test mesh
var test_mesh;
var test_mesh2;


var all_meshes = [];
var cached_models = [];
function initBuffers() {
    /*all_meshes.push(new gl_mesh(model_test.internal_mesh.positions,
                                model_test.internal_mesh.normals,
                                model_test.internal_mesh.uvCoords,
                                model_test.internal_mesh.indices));*/

    for(var x = 0; x < level.meshes.length; x++){
      if(level.meshes[x].indices.length > 0)
      {

        all_meshes.push(new gl_mesh(level.meshes[x].positions,
                                      level.meshes[x].normals,
                                      level.meshes[x].uvCoords,
                                      level.meshes[x].indices));
      }
    }

    // ===================
    // Load cached models
    for(var x = 0; x < level.cached_models.length; x++){
      if(level.cached_models[x].internal_mesh.indices.length > 0)
      {
        cached_models.push(new gl_mesh(level.cached_models[x].internal_mesh.positions,
                                    level.cached_models[x].internal_mesh.normals,
                                    level.cached_models[x].internal_mesh.uvCoords,
                                    level.cached_models[x].internal_mesh.indices));
      }
    }

    return;
    //NOTE: Testing only
    var y = 4;
    test_mesh = new gl_mesh(level.meshes[y].positions,
                                level.meshes[y].normals,
                                level.meshes[y].uvCoords,
                                level.meshes[y].indices);
    test_mesh2 = new gl_mesh(level.meshes[0].positions,
                                level.meshes[0].normals,
                                level.meshes[0].uvCoords,
                                level.meshes[0].indices);
}





//var location = vec3.create(0.0, 0.0, -10.0);;
var z = 0.0;

var viewMatrix = mat4.create();

function drawScene() {
  //===========================================================
  // Clear screen

  gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  mat4.perspective(50, gl.viewportWidth / gl.viewportHeight, 0.1, 100.0, pMatrix);

  mat4.identity(mvMatrix);

  //Model matrix
  mat4.translate(mvMatrix, [0.0, 0.0, 0.0]);

  mat4.rotate(mvMatrix, degToRad(xRot), [1, 0, 0]);
  mat4.rotate(mvMatrix, degToRad(yRot), [0, 1, 0]);

  //View matrix (global transform)
  mat4.identity(viewMatrix);
  //mat4.lookAt(viewMatrix, [0,0,6],[0,0,0],[0,1,0]);
  mat4.translate(viewMatrix, [0.0,-10.0,-30.0]);

  //Use test shader
  testShader.use();

  // Shader texture props
  //---------------------
  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, crateTexture);

  testShader.setInt("uSampler", 0);

  // Shader lighting props
  //----------------------
  testShader.setVector3f("uAmbientColor", 0.3, 0.3, 0.3);

  var lightingDirection = [-0.5, -0.4, -0.7];
  var adjustedLD = vec3.create();
  vec3.normalize(lightingDirection, adjustedLD);
  vec3.scale(adjustedLD, -1);
  testShader.setVector3fv("uLightingDirection", adjustedLD);

  testShader.setVector3f("uDirectionalColor", 1, 1, 1);

  // Shader matrices
  //----------------
  testShader.setMatrix4fv("uPMatrix", pMatrix);
  testShader.setMatrix4fv("uMVMatrix", mvMatrix);
  testShader.setMatrix4fv("viewMatrix", test_camera.getMatrix()); //View matrix (global transform)

  var normalMatrix = mat3.create();
  mat4.toInverseMat3(mvMatrix, normalMatrix);
  mat3.transpose(normalMatrix);
  testShader.setMatrix3fv("uNMatrix", normalMatrix);

  //test_mesh.draw();
  //test_mesh2.draw();
  //all_meshes[1].draw();

  for(var x = 0; x < all_meshes.length; x++)
  {
    all_meshes[x].draw();
  }

  gl.bindTexture(gl.TEXTURE_2D, modelTexture);

  for(var x = 0; x < level.models.length; x++)
  {
    //Reset model matrix
    mat4.identity(mvMatrix);

    //Translate model into position
    mat4.translate(mvMatrix, [
    level.models[x].position[0] * (2.5 / 100),
    level.models[x].position[2] * (2.5 / 100),
    level.models[x].position[1] * -(2.5 / 100)]);

    //Rotate the matrix to match
    mat4.rotate(mvMatrix, degToRad(level.models[x].rotation[0]), [0, 0, 1]);
    mat4.rotate(mvMatrix, degToRad(level.models[x].rotation[1]), [0, 1, 0]);
    mat4.rotate(mvMatrix, degToRad(level.models[x].rotation[2]), [1, 0, 0]);

    testShader.setMatrix4fv("uMVMatrix", mvMatrix);

    cached_models[level.models[x].modelID].draw();
  }
}


var lastTime = 0;

var fps_time_counter = 0.0;
frames = 0;

var fps_dom;

function animate() {

  var timeNow = new Date().getTime();
  if (lastTime != 0) {
    var elapsed = timeNow - lastTime;

    //vec3.translate(location, [0,0,-0.1]);
    z += elapsed / 1000.0;
    //xRot += (xSpeed * elapsed) / 1000.0;
    //yRot += (ySpeed * elapsed) / 1000.0;

    deltaTime = (timeNow - lastTime) * 0.1;

    fps_time_counter += elapsed;
    frames += 1;
    if(fps_time_counter > 1000.0){
      fps_dom.innerHTML = frames+"fps";
      fps_time_counter = 0.0;
      frames = 0;
    }
  }
  lastTime = timeNow;
}


function tick() {
  requestAnimFrame(tick);
  handleKeys();
  drawScene();
  animate();
}

function webGLStart() {
  var canvas = document.getElementById("glCanvas");

  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;


  fps_dom = document.getElementById("fps");
  initGL(canvas);
  initShaders(); //NOTE: Get rid of this
  initBuffers();
  initTexture();


  gl.clearColor(0.1, 0.1, 0.15, 1.0);
  gl.enable(gl.DEPTH_TEST);

  document.onkeydown = handleKeyDown;
  document.onkeyup = handleKeyUp;
  document.onmousemove = handleMouseMove;

  document.onclick = handleClick;
  document.onclickup = handleClickUp;

  tick();
}
