### MacOs OpenGL bootstrap

before to build

```bash
brew install glfw3
brew link glfw
```

```bash
brew install glew
brew link glew
```

```bash
brew install glm
brew link glm
```

### linux-deb bootstrap

before to build

```bash
sudo apt-get install libx11-dev mesa-common-dev libglu1-mesa-dev libglm-dev libglfw3-dev libglew-dev libfreetype6-dev
```

### dependencies

- CoreVideo
- Cocoa
- IOkit
- OpenGL
- glew
- glfw
- glm


### TODO

- light constant, linear, quadratic to intensity
- intensity replace far plane clipping constant
- multiple dir light integration
- spot light shadow
- cascade shadow map
- normal mapping
- ssao
- deferred rendering 