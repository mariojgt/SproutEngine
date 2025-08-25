# Contributing to Sprout Engine

Thank you for your interest in contributing to Sprout Engine! This document provides guidelines and information for contributors.

## 🌟 Ways to Contribute

### Code Contributions
- **Engine Features**: Core engine functionality, rendering, physics, audio
- **Editor Tools**: UI improvements, new panels, workflow enhancements
- **Asset Pipeline**: Import/export tools, format support, optimization
- **Platform Support**: Platform-specific implementations and testing

### Non-Code Contributions
- **Documentation**: Tutorials, API docs, guides, examples
- **Testing**: Bug reports, platform testing, performance testing
- **Design**: UI/UX improvements, icon design, themes
- **Community**: Discord moderation, forum support, social media

## 🚀 Getting Started

### Development Setup
1. Install Rust (latest stable): https://rustup.rs/
2. Clone the repository: `git clone https://github.com/mariojgt/SproutEngine.git`
3. Install dependencies: `cargo check`
4. Run the engine: `cargo run`
5. Run tests: `cargo test`

### Development Workflow
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Add tests if applicable
5. Ensure all tests pass: `cargo test`
6. Commit with clear messages: `git commit -m "Add amazing feature"`
7. Push to your fork: `git push origin feature/amazing-feature`
8. Open a Pull Request

## 📝 Code Guidelines

### Rust Style
- Follow the [Rust API Guidelines](https://rust-lang.github.io/api-guidelines/)
- Use `cargo fmt` for formatting
- Use `cargo clippy` for linting
- Write documentation for public APIs
- Add examples in doc comments when helpful

### Commit Messages
- Use clear, descriptive commit messages
- Start with a capital letter
- Use imperative mood ("Add feature" not "Added feature")
- Keep the first line under 50 characters
- Add detailed description if needed

Example:
```
Add material editor with PBR workflow

- Implement material property editing
- Add texture slot management
- Support for albedo, normal, metallic maps
- Include material preview system
```

### Code Organization
- Keep modules focused and cohesive
- Use descriptive names for functions and variables
- Add comments for complex logic
- Separate editor code from engine core
- Follow the existing architecture patterns

## 🧪 Testing

### Testing Requirements
- Write unit tests for new functionality
- Add integration tests for complex features
- Test on multiple platforms when possible
- Include performance tests for critical paths

### Running Tests
```bash
# Run all tests
cargo test

# Run specific test
cargo test test_name

# Run with output
cargo test -- --nocapture

# Run doctests
cargo test --doc
```

## 📋 Issue Guidelines

### Bug Reports
When reporting bugs, include:
- Sprout Engine version
- Operating system and version
- Rust version (`rustc --version`)
- Steps to reproduce
- Expected vs actual behavior
- Screenshots/videos if applicable
- Minimal reproduction case

### Feature Requests
When requesting features:
- Describe the use case clearly
- Explain why it's important
- Consider implementation complexity
- Look for existing similar requests
- Be open to alternative solutions

## 🎯 Priority Areas

### High Priority
- Core editor stability and performance
- Asset pipeline improvements
- Cross-platform compatibility
- Documentation and examples

### Medium Priority
- Advanced rendering features
- Physics integration
- Audio system enhancements
- Visual scripting system

### Future
- VR/AR support
- Console platform support
- Advanced AI tools
- Collaborative editing

## 🏆 Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes for significant contributions
- Social media shoutouts
- Special contributor role in Discord

## 💬 Communication

### Channels
- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: General questions, ideas
- **Discord**: Real-time chat, collaboration
- **Reddit**: Community discussions

### Getting Help
- Check existing issues and documentation first
- Ask questions in Discord #help channel
- Use GitHub Discussions for broader topics
- Tag maintainers for urgent issues

## 📜 Code of Conduct

We are committed to providing a welcoming and inclusive environment. All contributors must follow our [Code of Conduct](CODE_OF_CONDUCT.md).

### Expected Behavior
- Be respectful and constructive
- Welcome newcomers and help them learn
- Focus on what's best for the community
- Show empathy towards others

### Unacceptable Behavior
- Harassment or discrimination
- Trolling or insulting comments
- Personal attacks
- Spam or self-promotion

## 🔧 Development Tips

### Performance
- Profile before optimizing
- Consider memory allocations
- Use benchmarks for critical code
- Test on lower-end hardware

### Documentation
- Write docs as you code
- Include examples in documentation
- Keep README files updated
- Document breaking changes

### Architecture
- Follow ECS patterns where appropriate
- Keep systems loosely coupled
- Use events for communication
- Consider plugin architecture

## 📚 Resources

### Learning Rust
- [The Rust Book](https://doc.rust-lang.org/book/)
- [Rust by Example](https://doc.rust-lang.org/rust-by-example/)
- [Rustlings exercises](https://github.com/rust-lang/rustlings)

### Game Engine Development
- [Bevy Book](https://bevyengine.org/learn/book/)
- [Game Engine Architecture](https://www.gameenginebook.com/)
- [Real-Time Rendering](http://www.realtimerendering.com/)

### Graphics Programming
- [Learn OpenGL](https://learnopengl.com/)
- [GPU Gems series](https://developer.nvidia.com/gpugems)
- [Vulkan Tutorial](https://vulkan-tutorial.com/)

Thank you for contributing to Sprout Engine! Together, we're building the future of open-source game development. 🚀
