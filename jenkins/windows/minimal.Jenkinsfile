#!/usr/bin/env groovy

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
    stage('ConfigureBuildAndTest') {
      matrix {
        agent {
          dockerfile {
            dir 'jenkins/windows'
            filename 'Dockerfile'
            label 'win'
            additionalBuildArgs '--memory 2G --target minimal'
            args "--cpu-count 8 --memory 8G"
          }
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
                bat """
                      call "C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64 -vcvars_ver=14.3

                      cmake.exe "-GNinja" "-DBOOST_PREFIX=C:\\Boost" "-DBOOST_DYNAMIC=${env.SHARED_LIBS}" "-DWT_WRASTERIMAGE_IMPLEMENTATION=none" "-DBUILD_EXAMPLES=ON" "-DBUILD_TESTS=ON" "-DCONNECTOR_FCGI=OFF" "-DCONNECTOR_HTTP=ON" "-DEXAMPLES_CONNECTOR=wthttp" "-DENABLE_HARU=OFF" "-DENABLE_PANGO=OFF" "-DENABLE_POSTGRES=OFF" "-DENABLE_QT4=OFF" "-DENABLE_SQLITE=ON" "-DENABLE_SSL=OFF" "-DHTTP_WITH_ZLIB=OFF" -DMULTI_THREADED=ON -DSHARED_LIBS=${env.SHARED_LIBS} ..\\
                    """;
              }
            }
          }
          stage('Build') {
            steps {
              dir("build-shared-${env.SHARED_LIBS}") {
                bat """
                      call "C:\\BuildTools\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64 -vcvars_ver=14.3

                      ninja -v
                    """;
              }
            }
          }
          stage('Test') {
            steps {
              dir('test') {
                warnError('test.wt failed') {
                  bat """
                        set BuildDir=${env.WORKSPACE}\\build-shared-${env.SHARED_LIBS}
                        set Path=%BuildDir%\\src;C:\\Boost\\lib;%Path%

                        ..\\build-shared-${env.SHARED_LIBS}\\test\\test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/wt_shared_${env.SHARED_LIBS}_test_log.xml
                      """;
                }
              }
            }
          }
          stage('Test HTTP') {
            steps {
              dir('test') {
                warnError('test.http failed') {
                  bat """
                        set BuildDir=${env.WORKSPACE}\\build-shared-${env.SHARED_LIBS}
                        set Path=%BuildDir%\\src;%BuildDir%\\src\\http;C:\\Boost\\lib;%Path%

                        ..\\build-shared-${env.SHARED_LIBS}\\test\\test.http --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/http_shared_${env.SHARED_LIBS}_test_log.xml
                      """;
                }
              }
            }
          }
          stage('Test SQLite3') {
            steps {
              dir('test') {
                warnError('test.sqlite3 failed') {
                  bat """
                        set BuildDir=${env.WORKSPACE}\\build-shared-${env.SHARED_LIBS}
                        set Path=%BuildDir%\\src;%BuildDir%\\src\\Wt\\Dbo;%BuildDir%\\src\\Wt\\Dbo\\backend;C:\\Boost\\lib;%Path%

                        ..\\build-shared-${env.SHARED_LIBS}\\test\\test.sqlite3 --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/sqlite3_shared_${env.SHARED_LIBS}_test_log.xml
                      """;
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
