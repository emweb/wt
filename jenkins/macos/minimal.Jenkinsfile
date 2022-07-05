#!/usr/bin/env groovy

// This Jenkinsfile requires XCode and Homebrew to be installed, and the following dependencies
// are installed with Homebrew:
// brew install \
//   boost \
//   ccache \
//   cmake \
//   ninja

def wt_configure(Map args) {
  sh """eval "\$(/opt/homebrew/bin/brew shellenv)" && \
        cmake .. \
          -DCMAKE_BUILD_TYPE=RelWithDebInfo \
          -DCMAKE_CXX_STANDARD=17 \
          -DCMAKE_C_COMPILER_LAUNCHER=ccache \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
          -DBUILD_EXAMPLES=ON \
          -DBUILD_TESTS=ON \
          -DCONNECTOR_FCGI=OFF \
          -DCONNECTOR_HTTP=ON \
          -DDEBUG=OFF \
          -DEXAMPLES_CONNECTOR=wthttp \
          -DENABLE_HARU=OFF \
          -DENABLE_PANGO=OFF \
          -DENABLE_SQLITE=ON \
          -DENABLE_POSTGRES=OFF \
          -DENABLE_FIREBIRD=OFF \
          -DENABLE_MYSQL=OFF \
          -DENABLE_MSSQLSERVER=OFF \
          -DENABLE_QT4=OFF \
          -DENABLE_QT5=OFF \
          -DENABLE_QT6=OFF \
          -DENABLE_SSL=OFF \
          -DENABLE_OPENGL=OFF \
          -DHTTP_WITH_ZLIB=OFF \
          -DSHARED_LIBS=${args.shared} \
          -DMULTI_THREADED=ON \
          -GNinja"""
}

pipeline {
  environment {
    EMAIL = credentials('wt-dev-mail')
  }
  options {
    buildDiscarder logRotator(numToKeepStr: '20')
    disableConcurrentBuilds abortPrevious: true
  }
  triggers {
    pollSCM('H/5 * * * *')
  }
  agent none
  stages {
    stage("ConfigureBuildAndTest") {
      matrix {
        agent {
          label 'mac'
        }
        axes {
          axis {
            name 'SHARED_LIBS'
            values 'ON', 'OFF'
          }
        }
        stages {
          stage('Configure') {
            steps {
              dir("build-shared-${env.SHARED_LIBS}") {
                wt_configure(shared: env.SHARED_LIBS)
              }
            }
          }
          stage('Build') {
            steps {
              dir("build-shared-${env.SHARED_LIBS}") {
                sh "eval \"\$(/opt/homebrew/bin/brew shellenv)\" && ninja all examples/all -k 0"
              }
            }
          }
          stage('Test') {
            steps {
              dir('test') {
                warnError('test.wt failed') {
                  sh "../build-shared-${env.SHARED_LIBS}/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/wt_shared_${env.SHARED_LIBS}_test_log.xml"
                }
              }
            }
          }
          stage('Test HTTP') {
            steps {
              dir('test') {
                warnError('test.http failed') {
                  sh "../build-shared-${env.SHARED_LIBS}/test/test.http --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/http_shared_${env.SHARED_LIBS}_test_log.xml"
                }
              }
            }
          }
          stage('Test SQLite3') {
            steps {
              dir('test') {
                warnError('test.sqlite3 failed') {
                  sh "../build-shared-${env.SHARED_LIBS}/test/test.sqlite3 --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/sqlite3_shared_${env.SHARED_LIBS}_test_log.xml"
                }
              }
            }
          }
        }
        post {
          success {
            junit '*_test_log.xml';
          }
          unstable {
            junit '*_test_log.xml';
          }
          cleanup {
            cleanWs cleanWhenNotBuilt: false,
                    deleteDirs: true,
                    disableDeferredWipeout: true,
                    notFailBuild: true,
                    patterns: [[pattern: 'build-*', type: 'INCLUDE']];
          }
        }
      }
    }
  }
  post {
    failure {
      mail to: env.EMAIL,
           subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
           body: "Something is wrong with ${env.BUILD_URL}";
    }
    unstable {
      mail to: env.EMAIL,
           subject: "Unstable Pipeline: ${currentBuild.fullDisplayName}",
           body: "Something is wrong with ${env.BUILD_URL}";
    }
  }
}
