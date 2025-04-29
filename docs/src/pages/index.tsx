import React, { JSX, useEffect, useRef } from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import CodeBlock from '@theme/CodeBlock';
import Heading from '@theme/Heading';
import { FiZap, FiCpu, FiServer, FiLayers, FiGithub, FiArrowRight, FiCode, FiShield } from 'react-icons/fi';
import { HiOutlineChip } from 'react-icons/hi';
import { SlSpeedometer } from 'react-icons/sl';
import { BsCodeSquare, BsGit } from 'react-icons/bs';

import styles from './index.module.css';

const features = [
  {
    title: 'High Performance',
    description: 'Process over 100,000 requests per second with ultra-low latency. Engineered for speed from the ground up.',
    icon: <SlSpeedometer className={styles.featureIcon} />,
  },
  {
    title: 'Modern C++ (17/20)',
    description: 'Built with the latest C++ features for safer, cleaner, and more maintainable code.',
    icon: <FiCode className={styles.featureIcon} />,
  },
  {
    title: 'Express-Inspired API',
    description: 'Familiar syntax makes web development in C++ accessible, even to those coming from JavaScript.',
    icon: <BsCodeSquare className={styles.featureIcon} />,
  },
  {
    title: 'Memory Efficient',
    description: 'Only a fraction of the memory footprint compared to JavaScript or Python frameworks.',
    icon: <HiOutlineChip className={styles.featureIcon} />,
  },
  {
    title: 'Built-in Middleware',
    description: 'Easily extend request processing with global or route-specific middleware for modular architecture.',
    icon: <FiLayers className={styles.featureIcon} />,
  },
  {
    title: 'Production Ready',
    description: 'Comprehensive error handling, logging, and security features for robust production deployments.',
    icon: <FiShield className={styles.featureIcon} />,
  },
];

const codeExample = `#include <boson/boson.hpp>

int main() {
    boson::initialize();
    
    boson::Server app;
    
    // Simple middleware
    app.use([](const boson::Request& req, boson::Response& res, boson::NextFunction& next) {
        std::cout << "[" << req.method() << "] " << req.path() << std::endl;
        next();
    });
    
    // JSON API endpoint
    app.get("/api/users/:id", [](const boson::Request& req, boson::Response& res) {
        std::string id = req.param("id");
        res.jsonObject({
            {"id", id},
            {"name", "User " + id},
            {"email", "user" + id + "@example.com"}
        });
    });
    
    app.configure(3000, "127.0.0.1");
    return app.listen();
}`;

function HeroSection() {
  return (
    <header className={styles.hero}>
      <div className={styles.heroInner}>
        <div className={styles.heroContent}>
          <div className={styles.heroLogo}>
            <FiZap className={styles.logoIcon} />
            <span className={styles.logoText}>Boson</span>
          </div>
          <Heading as="h1" className={styles.heroTitle}>
            High-Performance Web Framework for Modern C++
          </Heading>
          <p className={styles.heroSubtitle}>
            Build blazing-fast APIs and web applications with the raw power of C++ and an elegant, Express.js-inspired syntax
          </p>
          <div className={styles.heroVideo}>
            <video width="100%" controls>
              <source src="https://gkh-images.s3.amazonaws.com/boson.mov" type="video/mp4" />
              Your browser does not support the video tag.
            </video>
          </div>
          <div className={styles.heroButtons}>
            <Link
              className={clsx('button button--primary button--lg', styles.primaryButton)}
              to="/docs/getting-started/installation">
              Get Started
            </Link>
            <Link
              className={clsx('button button--secondary button--lg', styles.secondaryButton)}
              to="/docs">
              Documentation
            </Link>
            <Link
              className={clsx('button button--outline button--lg', styles.githubButton)}
              href="https://github.com/S1D007/boson">
              <FiGithub className={styles.buttonIcon} />
              GitHub
            </Link>
          </div>
        </div>
        <div className={styles.heroStats}>
          <div className={styles.statCard}>
            <div className={styles.statNumber}>100K+</div>
            <div className={styles.statLabel}>Requests/sec</div>
          </div>
          <div className={styles.statCard}>
            <div className={styles.statNumber}>&lt;1ms</div>
            <div className={styles.statLabel}>Latency</div>
          </div>
          <div className={styles.statCard}>
            <div className={styles.statNumber}>3MB</div>
            <div className={styles.statLabel}>Memory</div>
          </div>
          <div className={styles.statCard}>
            <div className={styles.statNumber}>C++17</div>
            <div className={styles.statLabel}>Standard</div>
          </div>
        </div>
      </div>
    </header>
  );
}

function FeatureSection() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className={styles.sectionHeader}>
          <Heading as="h2" className={styles.sectionTitle}>Framework Features</Heading>
          <p className={styles.sectionSubtitle}>
            Boson provides a comprehensive set of features designed for high-performance web development
          </p>
        </div>
        <div className={styles.featuresGrid}>
          {features.map((feature, idx) => (
            <div key={idx} className={styles.featureItem}>
              <div className={styles.featureIconWrapper}>
                {feature.icon}
              </div>
              <h3 className={styles.featureTitle}>{feature.title}</h3>
              <p className={styles.featureDescription}>{feature.description}</p>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
}

function CodeSection() {
  return (
    <section className={styles.codeSection}>
      <div className="container">
        <div className={styles.sectionHeader}>
          <Heading as="h2" className={styles.sectionTitle}>Elegant, Express-Inspired API</Heading>
          <p className={styles.sectionSubtitle}>
            Write beautiful, maintainable C++ code that feels like modern web frameworks while keeping all the performance benefits
          </p>
        </div>
        <div className={styles.codeContainer}>
          <div className={styles.codeWindow}>
            <div className={styles.codeWindowHeader}>
              <div className={styles.codeWindowButton}></div>
              <div className={styles.codeWindowButton}></div>
              <div className={styles.codeWindowButton}></div>
              <span className={styles.codeWindowTitle}>app.cpp</span>
            </div>
            <div className={styles.codeWindowContent}>
              <CodeBlock language="cpp">
                {codeExample}
              </CodeBlock>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}

function PerformanceSection() {
  return (
    <section className={styles.performanceSection}>
      <div className="container">
        <div className={styles.sectionHeader}>
          <Heading as="h2" className={styles.sectionTitle}>CRUSHING THE COMPETITION</Heading>
          <p className={styles.sectionSubtitle}>
            While other frameworks struggle with basic loads, Boson delivers raw, unmatched power for the most demanding applications
          </p>
        </div>
        
        <div className={styles.comparisonIntro}>
          <p className={styles.comparisonText}>
            <span className={styles.textHighlight}>Face it</span> — your current tech stack is holding you back. 
            Why settle for frameworks that <span className={styles.textDanger}>choke under pressure</span> when Boson handles 
            what others can only <span className={styles.textHighlight}>dream of achieving?</span>
          </p>
        </div>
        
        <div className={styles.performanceMetrics}>
          <div className={styles.metricBox}>
            <div className={styles.metricValue}>20x</div>
            <div className={styles.metricLabel}>FASTER THAN PYTHON</div>
            <div className={styles.metricSubtext}>While they're still processing the first request, you've already handled twenty with Boson.</div>
          </div>
          
          <div className={styles.metricBox}>
            <div className={styles.metricValue}>8x</div>
            <div className={styles.metricLabel}>FASTER THAN NODE.JS</div>
            <div className={styles.metricSubtext}>Stop waiting for your event loop and experience true performance without compromise.</div>
          </div>
          
          <div className={styles.metricBox}>
            <div className={styles.metricValue}>3x</div>
            <div className={styles.metricLabel}>FASTER THAN GO</div>
            <div className={styles.metricSubtext}>Even Go's celebrated speed can't match C++'s raw power in Boson's optimized architecture.</div>
          </div>
        </div>
        
        <div className={styles.memorySavings}>
          <h3 className={styles.memorySavingsTitle}>And Memory Usage? It's Not Even a Contest.</h3>
          <div className={styles.memorySavingsContent}>
            <div className={styles.memorySavingsIcon}>
              <HiOutlineChip className={styles.largeIcon} />
            </div>
            <div className={styles.memorySavingsText}>
              <p>While Python and Node.js bloat to <span className={styles.textDanger}>hundreds of MB</span> of RAM, 
              Boson delivers the same functionality in just <span className={styles.textSuccess}>3MB</span>.</p>
              <p>Cut your server costs by 90%. Run 10x more instances on the same hardware. Scale further than ever before.</p>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}

function UseCasesSection() {
  const useCases = [
    {
      title: 'High-Volume APIs',
      description: 'Handle millions of requests with minimal resources. Perfect for high-traffic APIs where every millisecond counts.',
      icon: <FiServer />,
      color: '#0066ff',
    },
    {
      title: 'Gaming Backends',
      description: 'Process real-time events with ultra-low latency for responsive multiplayer experiences and game services.',
      icon: <FiCpu />,
      color: '#9900ff',
    },
    {
      title: 'Trading Systems',
      description: 'Execute time-sensitive financial operations with the reliability and speed that C++ is known for.',
      icon: <FiZap />,
      color: '#ff3300',
    },
    {
      title: 'IoT Applications',
      description: 'Optimize for constrained environments where resources are limited but performance demands are high.',
      icon: <HiOutlineChip />,
      color: '#00cc66',
    },
  ];

  return (
    <section className={styles.useCasesSection}>
      <div className="container">
        <div className={styles.sectionHeader}>
          <Heading as="h2" className={styles.sectionTitle}>Perfect For High-Performance Applications</Heading>
          <p className={styles.sectionSubtitle}>
            Boson excels in scenarios where performance, efficiency, and reliability are critical
          </p>
        </div>
        <div className={styles.useCasesGrid}>
          {useCases.map((useCase, idx) => (
            <div key={idx} className={styles.useCaseCard}
                 style={{ '--card-accent-color': useCase.color } as React.CSSProperties}>
              <div className={styles.useCaseIcon}>{useCase.icon}</div>
              <h3 className={styles.useCaseTitle}>{useCase.title}</h3>
              <p className={styles.useCaseDescription}>{useCase.description}</p>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
}

function GettingStartedSection() {
  return (
    <section className={styles.gettingStartedSection}>
      <div className="container">
        <div className={styles.sectionHeader}>
          <Heading as="h2" className={styles.sectionTitle}>Get Started in Minutes</Heading>
          <p className={styles.sectionSubtitle}>
            Boson makes it easy to start building high-performance web applications with a simple setup process
          </p>
        </div>
        <div className={styles.stepsContainer}>
          <div className={styles.step}>
            <div className={styles.stepNumber}>1</div>
            <div className={styles.stepContent}>
              <h3 className={styles.stepTitle}>Install Boson</h3>
              <div className={styles.stepCommand}>
                <CodeBlock language="bash">
                  curl -fsSL https://raw.githubusercontent.com/S1D007/boson/main/install.sh | bash
                </CodeBlock>
              </div>
            </div>
          </div>
          <div className={styles.step}>
            <div className={styles.stepNumber}>2</div>
            <div className={styles.stepContent}>
              <h3 className={styles.stepTitle}>Create a New Project</h3>
              <div className={styles.stepCommand}>
                <CodeBlock language="bash">
                  boson new my-project &&
                  cd my-project
                </CodeBlock>
              </div>
            </div>
          </div>
          <div className={styles.step}>
            <div className={styles.stepNumber}>3</div>
            <div className={styles.stepContent}>
              <h3 className={styles.stepTitle}>Run Your Server</h3>
              <div className={styles.stepCommand}>
                <CodeBlock language="bash">
                  boson run -w
                </CodeBlock>
              </div>
            </div>
          </div>
        </div>
        <div className={styles.ctaContainer}>
          <Link
            className={clsx('button button--primary button--lg', styles.primaryButton)}
            to="/docs/getting-started/quickstart">
            Follow the Quickstart Guide <FiArrowRight className={styles.buttonIconRight} />
          </Link>
        </div>
      </div>
    </section>
  );
}

function FooterBanner() {
  return (
    <section className={styles.footerBanner}>
      <div className="container">
        <div className={styles.footerBannerContent}>
          <Heading as="h2" className={styles.footerBannerTitle}>Ready to supercharge your web applications?</Heading>
          <p className={styles.footerBannerText}>
            Join the community of developers building high-performance applications with Boson
          </p>
          <div className={styles.footerBannerButtons}>
            <Link
              className={clsx('button button--lg', styles.whiteButton)}
              to="/docs/getting-started/installation">
              Get Started Now
            </Link>
            <Link
              className={clsx('button button--outline button--lg', styles.githubButtonAlt)}
              href="https://github.com/S1D007/boson">
              <BsGit className={styles.buttonIcon} />
              Star on GitHub
            </Link>
          </div>
        </div>
      </div>
    </section>
  );
}

export default function Home(): JSX.Element {
  const { siteConfig } = useDocusaurusContext();
  return (
    <Layout
      title={`${siteConfig.title} - High-Performance C++ Web Framework`}
      description="Boson is a blazing-fast, modern C++ web framework for building high-performance APIs and web applications with an elegant, Express.js-inspired syntax.">
      <HeroSection />
      <FeatureSection />
      <CodeSection />
      <PerformanceSection />
      <UseCasesSection />
      <GettingStartedSection />
      <FooterBanner />
    </Layout>
  );
}
