#!/usr/bin/env groovy

def user_id
def user_name
def group_id
def group_name
def container_ccache_dir
def host_ccache_dir

node('wt') {
  user_id = sh(returnStdout: true, script: 'id -u').trim()
  user_name = sh(returnStdout: true, script: 'id -un').trim()
  group_id = sh(returnStdout: true, script: 'id -g').trim()
  group_name = sh(returnStdout: true, script: 'id -gn').trim()
  container_ccache_dir = "/home/${user_name}/.ccache"
  host_ccache_dir = "/local/home/${user_name}/.ccache"
}

def wt_configure() {
  sh """cmake .. \
          -DCMAKE_BUILD_TYPE=RelWithDebInfo \
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
          -DENABLE_POSTGRES=OFF \
          -DENABLE_QT4=OFF \
          -DENABLE_QT5=OFF \
          -DENABLE_SQLITE=ON \
          -DENABLE_SSL=OFF \
          -DHTTP_WITH_ZLIB=OFF \
          -DSHARED_LIBS=ON \
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
    pollSCM('@midnight')
  }
  agent none
  stages {
    stage("ConfigureBuildAndTest") {
      matrix {
        axes {
          axis {
            name 'OS'
            values 'debian10', 'rocky8', 'ubuntu2004'
          }
        }
        agent {
          dockerfile {
            label 'wt'
            dir 'jenkins/linux/minver'
            filename "${env.OS}.Dockerfile"
            args "--env CCACHE_DIR=${container_ccache_dir} --env CCACHE_MAXSIZE=20G --volume ${host_ccache_dir}:${container_ccache_dir}:z"
            additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                   --build-arg USER_NAME=${user_name} \
                                   --build-arg GROUP_ID=${group_id} \
                                   --build-arg GROUP_NAME=${group_name}"""
          }
        }
        stages {
          stage('Configure') {
            steps {
              dir ("build-${env.OS}") {
                wt_configure()
              }
            }
          }
          stage('Build') {
            steps {
              dir ("build-${env.OS}") {
                sh "ninja all examples/all -k 0"
              }
            }
          }
          stage('Test') {
            steps {
              dir('test') {
                warnError('test.wt failed') {
                  sh "../build-${env.OS}/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/wt_${env.OS}_test_log.xml"
                }
              }
            }
          }
          stage('Test HTTP') {
            steps {
              dir('test') {
                warnError('test.http failed') {
                  sh "../build-${env.OS}/test/test.http --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/http_${env.OS}_test_log.xml"
                }
              }
            }
          }
          stage('Test SQLite3') {
            steps {
              dir('test') {
                warnError('test.sqlite3 failed') {
                  sh "../build-${env.OS}/test/test.sqlite3 --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/sqlite3_${env.OS}_test_log.xml"
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
