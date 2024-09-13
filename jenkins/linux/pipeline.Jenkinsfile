#!/usr/bin/env groovy

def user_id
def user_name
def group_id
def group_name
def container_ccache_dir
def host_ccache_dir

def thread_count = 10

node('docker') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
    container_ccache_dir = "/home/${user_name}/.ccache"
    host_ccache_dir = "/local/home/${user_name}/.ccache"
}

def wt_configure(Map args) {
    sh """cmake .. \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DDEBUG=ON \
            -DEXAMPLES_CONNECTOR=wthttp \
            -DENABLE_HARU=ON \
            -DHARU_PREFIX=/opt/haru \
            -DENABLE_PANGO=ON \
            -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick \
            -DENABLE_QT4=OFF \
            -DENABLE_QT5=OFF \
            -DENABLE_QT6=ON \
            -DENABLE_SQLITE=ON \
            -DUSE_SYSTEM_SQLITE3=ON \
            -DENABLE_POSTGRES=OFF \
            -DENABLE_FIREBIRD=OFF \
            -DENABLE_MYSQL=OFF \
            -DENABLE_MSSQLSERVER=OFF \
            -DENABLE_OPENGL=ON \
            -DENABLE_SAML=ON \
            -DENABLE_SSL=ON \
            -DHTTP_WITH_ZLIB=ON \
            -DSHARED_LIBS=ON \
            -DMULTI_THREADED=${args.mt}"""
}

pipeline {
    environment {
        EMAIL = credentials('wt-dev-mail')
    }
    options {
        buildDiscarder logRotator(numToKeepStr: '20')
        disableConcurrentBuilds abortPrevious: true

        gitLabConnection('Gitlab')
        gitlabBuilds(builds: ['Format JavaScript', 'Build - Single Threaded', 'Build - Multi Threaded', 'Tests', 'Tests - Sqlite3'])
    }
    agent {
        dockerfile {
            label 'docker'
            dir 'jenkins/linux'
            filename 'pipeline.Dockerfile'
            args "--env CCACHE_DIR=${container_ccache_dir} --env CCACHE_MAXSIZE=20G --volume ${host_ccache_dir}:${container_ccache_dir}:z"
            additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                   --build-arg USER_NAME=${user_name} \
                                   --build-arg GROUP_ID=${group_id} \
                                   --build-arg GROUP_NAME=${group_name} \
                                   --build-arg THREAD_COUNT=${thread_count}"""
        }
    }
    stages {
        stage('Check JS') {
            stages {
                stage('pnpm install') {
                    steps {
                        updateGitlabCommitStatus name: 'Format JavaScript', state: 'running'
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
            post {
                failure {
                    updateGitlabCommitStatus name: 'Format JavaScript', state: 'failed'
                }
                success {
                    updateGitlabCommitStatus name: 'Format JavaScript', state: 'success'
                }
            }
        }
        stage('Building application - Single threaded') {
            steps {
                updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'running'
                dir('build-st') {
                    wt_configure(mt: 'OFF')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
            }
            post {
                failure {
                    updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'failed'
                }
                success {
                    updateGitlabCommitStatus name: 'Build - Single Threaded', state: 'success'
                }
            }
        }
        stage('Building application - Multi threaded') {
            steps {
                updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'running'
                dir('build-mt') {
                    wt_configure(mt: 'ON')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
            }
            post {
                failure {
                    updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'failed'
                }
                success {
                    updateGitlabCommitStatus name: 'Build - Multi Threaded', state: 'success'
                }
            }
        }
        stage('Tests') {
            steps {
                updateGitlabCommitStatus name: 'Tests', state: 'running'
                dir('test') {
                    warnError('st test.wt failed') {
                        sh "../build-st/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_test_log.xml"
                    }
                }
                dir('test') {
                    warnError('mt test.wt failed') {
                        sh "../build-mt/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_test_log.xml"
                    }
                }
            }
            post {
                failure {
                    updateGitlabCommitStatus name: 'Tests', state: 'failed'
                }
                success {
                    updateGitlabCommitStatus name: 'Tests', state: 'success'
                }
            }
        }
        stage('Test SQLite3') {
            steps {
                updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'running'
                dir('test') {
                    warnError('st test.sqlite3 failed') {
                        sh "../build-st/test/test.sqlite3 --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_sqlite3_test_log.xml"
                    }
                }
                dir('test') {
                    warnError('mt test.sqlite3 failed') {
                        sh "../build-mt/test/test.sqlite3 --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_sqlite3_test_log.xml"
                    }
                }
            }
            post {
                failure {
                    updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'failed'
                }
                success {
                    updateGitlabCommitStatus name: 'Tests - Sqlite3', state: 'success'
                }
            }
        }
    }
    post {
        always {
            junit allowEmptyResults: true, testResults: '*_test_log.xml'
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
