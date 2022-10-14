#!/usr/bin/env groovy

def user_id
def user_name
def group_id
def group_name
def container_ccache_dir
def host_ccache_dir

def thread_count = 10

node('wt') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
    container_ccache_dir = "/home/${user_name}/.ccache"
    host_ccache_dir = "/local/home/${user_name}/.ccache"
    container_pnpm_store_dir = "/home/${user_name}/.pnpm-store"
    host_pnpm_store_dir = "/local/home/${user_name}/.pnpm-store"
}

def wt_configure(Map args) {
    sh """cmake .. \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DDEBUG=ON \
            -DEXAMPLES_CONNECTOR=wthttp \
            -DENABLE_HARU=OFF \
            -DENABLE_PANGO=OFF \
            -DENABLE_POSTGRES=OFF \
            -DENABLE_QT4=OFF \
            -DENABLE_QT5=OFF \
            -DENABLE_SQLITE=ON \
            -DENABLE_SSL=OFF \
            -DHTTP_WITH_ZLIB=OFF \
            -DSHARED_LIBS=OFF \
            -DBOOST_PREFIX=/opt/boost \
            -DMULTI_THREADED=${args.mt}"""
}

pipeline {
    environment {
        EMAIL = credentials('wt-dev-mail')
    }
    options {
        buildDiscarder logRotator(numToKeepStr: '20')
        disableConcurrentBuilds abortPrevious: true
    }
    agent {
        dockerfile {
            label 'wt'
            dir 'jenkins/linux'
            filename 'minimal.Dockerfile'
            args """--env CCACHE_DIR=${container_ccache_dir} \
                    --env CCACHE_MAXSIZE=20G \
                    --volume ${host_ccache_dir}:${container_ccache_dir}:z
                    --volume ${host_pnpm_store_dir}:${container_pnpm_store_dir}:z"""
            additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                   --build-arg USER_NAME=${user_name} \
                                   --build-arg GROUP_ID=${group_id} \
                                   --build-arg GROUP_NAME=${group_name} \
                                   --build-arg THREAD_COUNT=${thread_count}"""
        }
    }
    triggers {
        pollSCM('H/5 * * * *')
    }
    stages {
        stage('Check JS') {
            stages {
                stage('pnpm install') {
                    steps {
                        dir('src/js') {
                            sh '''#!/bin/bash
                              export PNPM_HOME="${HOME}/.local/share/pnpm"
                              export PATH="${PNPM_HOME}:${PATH}"
                              pnpm install
                            '''
                        }
                    }
                }
                stage('Check formatting') {
                    steps {
                        dir('src/js') {
                            sh '''#!/bin/bash
                              export PNPM_HOME="${HOME}/.local/share/pnpm"
                              export PATH="${PNPM_HOME}:${PATH}"
                              pnpm run checkfmt
                            '''
                        }
                    }
                }
                stage('Linting') {
                    steps {
                        dir('src/js') {
                            sh '''#!/bin/bash
                              export PNPM_HOME="${HOME}/.local/share/pnpm"
                              export PATH="${PNPM_HOME}:${PATH}"
                              pnpm run lint-junit
                            '''
                        }
                    }
                }
            }
        }
        stage('Single-threaded') {
            steps {
                dir('build-st') {
                    wt_configure(mt: 'OFF')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('st test.wt failed') {
                        sh "../build-st/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_test_log.xml"
                    }
                }
            }
        }
        stage('Multi-threaded') {
            steps {
                dir('build-mt') {
                    wt_configure(mt: 'ON')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('mt test.wt failed') {
                        sh "../build-mt/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_test_log.xml"
                    }
                }
            }
        }
    }
    post {
        always {
            junit 'eslint-report.xml'
            junit '*_test_log.xml'
        }
        cleanup {
            cleanWs()
        }
        failure {
            mail to: env.EMAIL,
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
        unstable {
            mail to: env.EMAIL,
                 subject: "Unstable Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
    }
}
