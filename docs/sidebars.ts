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
    'architecture',
    {
      type: 'category',
      label: 'Core Concepts',
      link: {
        type: 'doc',
        id: 'core-concepts/index',
      },
      items: [
        'core-concepts/entities',
        'core-concepts/components',
        'core-concepts/systems',
        'core-concepts/resources',
        'core-concepts/queries',
        'core-concepts/commands',
      ],
    },
    {
      type: 'category',
      label: 'Advanced Features',
      link: {
        type: 'doc',
        id: 'advanced/index',
      },
      items: [
        'advanced/events',
        'advanced/hierarchies',
        'advanced/run-conditions',
        'advanced/schedules',
      ],
    },
    'storage',
    {
      type: 'category',
      label: 'Examples',
      link: {
        type: 'doc',
        id: 'examples/index',
      },
      items: [
        'examples/bouncing-balls',
        'examples/system-hierarchy',
        'examples/event-communication',
      ],
    },
    {
      type: 'category',
      label: 'API Reference',
      link: {
        type: 'doc',
        id: 'api/index',
      },
      items: [
        'api/scene',
        'api/commands',
        'api/entity-commands',
        'api/query',
        'api/events',
      ],
    },
  ],
};

export default sidebars;
