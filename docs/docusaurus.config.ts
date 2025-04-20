import { themes as prismThemes } from "prism-react-renderer";
import type { Config } from "@docusaurus/types";
import type * as Preset from "@docusaurus/preset-classic";

// This runs in Node.js - Don't use client-side code here (browser APIs, JSX...)

const config: Config = {
  title: "Boson Framework",
  tagline: "Modern C++ web framework for high-performance applications",
  favicon: "img/favicon.ico",

  url: "https://bosonframework.vercel.app/",
  baseUrl: "/",

  organizationName: "boson",
  projectName: "boson-framework",

  onBrokenLinks: "log",
  onBrokenMarkdownLinks: "warn",

  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  presets: [
    [
      "classic",
      {
        docs: {
          sidebarPath: "./sidebars.ts",
          editUrl: "https://github.com/S1D007/Boson-Docs/blob/main/",
        },
        theme: {
          customCss: "./src/css/custom.css",
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    image: "img/boson-social-card.jpg",
    navbar: {
      title: "Boson",
      logo: {
        alt: "Boson Framework Logo",
        src: "img/logo.svg",
      },
      items: [
        {
          type: "docSidebar",
          sidebarId: "tutorialSidebar",
          position: "left",
          label: "Documentation",
        },
        {
          href: "https://github.com/S1D007/boson",
          label: "GitHub",
          position: "right",
        },
      ],
    },
    footer: {
      style: "light",
      links: [
        {
          title: "Docs",
          items: [
            {
              label: "Getting Started",
              to: "/getting-started",
            },
            {
              label: "Core Concepts",
              to: "/core-concepts",
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} Boson Framework. Built with ❤️ by Siddhant in India(🇮🇳).`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
      additionalLanguages: ["cpp", "cmake", "bash", "json"],
    },
    algolia: {
      appId: "YOUR_APP_ID",
      apiKey: "YOUR_API_KEY",
      indexName: "boson",
      contextualSearch: true,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
