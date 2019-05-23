module.exports = {
  base: "/wrenbind17/",
  title: "Wren Bind c++17",
  description: "Wren language binding library for C++17",
  themeConfig: {
    sidebar: "auto",
    nav: [
      { text: "Home", link: "/" },
      { text: "Tutorial", link: "/docs/tutorial.md" },
      { text: "Modules", link: "/docs/doxygen/modules" },
      {
        text: 'Classes',
        items: [
          { text: 'Class List', link: '/docs/doxygen/annotated' },
          { text: 'Class Index', link: '/docs/doxygen/classes' },
          { text: 'Class Hierarchy', link: '/docs/doxygen/hierarchy' },
          { text: 'Class Members', link: '/docs/doxygen/class_members' },
          { text: 'Class Member Functions', link: '/docs/doxygen/class_member_functions' },
          { text: 'Class Member Variables', link: '/docs/doxygen/class_member_variables' },
          { text: 'Class Member Typedefs', link: '/docs/doxygen/class_member_typedefs' },
          { text: 'Class Member Enumerations', link: '/docs/doxygen/class_member_enums' }
        ]
      },
      {
        text: 'Namespaces',
        items: [
          { text: 'Namespace List', link: '/docs/doxygen/namespaces' },
          { text: 'Namespace Members', link: '/docs/doxygen/namespace_members' },
          { text: 'Namespace Member Functions', link: '/docs/doxygen/namespace_member_functions' },
          { text: 'Namespace Member Variables', link: '/docs/doxygen/namespace_member_variables' },
          { text: 'Namespace Member Typedefs', link: '/docs/doxygen/namespace_member_typedefs' },
          { text: 'Namespace Member Enumerations', link: '/docs/doxygen/namespace_member_enums' }
        ]
      },
      { text: 'Files', link: '/docs/doxygen/files' },
      { text: 'Pages', link: '/docs/doxygen/pages' },
      { text: 'Bugs', link: '/docs/doxygen/bug' }
    ]
  }
};
