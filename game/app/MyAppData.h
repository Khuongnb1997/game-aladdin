#ifndef __MY_APP_DATA_H__
#define __MY_APP_DATA_H__

#include <aladdin.h>

ALA_CLASS_HEADER_1( MyAppData, ala::GameResource )
private:
  int _numberOfLevels;
  int _currentLevel;
  int _currentCheckpoint;
  int _aladdinLives;
  int _aladdinApples;
  int _retryTimes;
  std::vector<std::string> _levelScenes;
  bool _sfxOn;
  bool _musicOn;
  int _difficulty;

public:
  MyAppData( const int numberOfLevels, const int currentLevel, const int aladdinLives, const int retryTimes,
             const std::vector<std::string>& levelScenes )
    : GameResource( "My App Data" ), _numberOfLevels( numberOfLevels ),
      _currentLevel( currentLevel ), _currentCheckpoint( 0 ), _aladdinLives( aladdinLives ), _aladdinApples( 3 ),
      _retryTimes( retryTimes ),
      _levelScenes( levelScenes ), _sfxOn( true ), _musicOn( true ), _difficulty( 2 ) {}

  int getNumberOfLevels() const;

  int getAladdinLives() const;

  void setAladdinLives( const int aladdinLives );

  int getAladdinApples() const;

  void setAladdinApples( const int apples );

  int getCurrentLevel() const;

  void setCurrentLevel( const int currentLevel );

  int getRetryTimes() const;

  void setRetryTimes( const int retryTimes );

  int getCurrentCheckpoint() const;

  void setCurrentCheckpoint( const int currentCheckPoint );

  std::string getSceneName( const int level );

  bool isSfxOn() const;

  void setSfxOn( const bool sfxOn );

  bool isMusicOn() const;

  void setMusicOn( const bool musicOn );

  int getDifficulty() const;

  void setDifficulty( const int difficulty );

protected:
  void onLoad() override;

  void onRelease() override;
};

#endif //!__MY_APP_DATA_H__
