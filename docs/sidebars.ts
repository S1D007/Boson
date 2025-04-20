import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

// This runs in Node.js - Don't use client-side code here (browser APIs, JSX...)

/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */
const sidebars: SidebarsConfig = {
  tutorialSidebar: [
    'intro',
    {
      type: 'category',
      collapsible: false,
      label: 'Introduction',
      items: [
        'introduction/what-is-boson',
        'introduction/why-boson',
        'introduction/features',
        'introduction/architecture',
      ],
    },
    {
      type: 'category',
      label: 'Getting Started',
      collapsible: false,
      items: [
        'getting-started/installation',
        'getting-started/prerequisites',
        'getting-started/using-cli',
        'getting-started/quickstart',
        'getting-started/hello-world',
        'getting-started/project-structure',
      ],
    },
    {
      type: 'category',
      collapsible: false,
      label: 'Core Concepts',
      items: [
        'core-concepts/server',
        'core-concepts/routing',
        'core-concepts/controllers',
        'core-concepts/middleware',
        'core-concepts/request-response',
        'core-concepts/error-handling',
      ],
    },
    {
      type: 'category',
      label: "Examples",
      collapsible: false,
      items: [
        'examples/rest-api',
      ],
    }
  ],
};

export default sidebars;
