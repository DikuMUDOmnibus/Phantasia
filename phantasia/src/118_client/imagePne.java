import java.awt.*;

public class imagePne extends Canvas implements constants {

    pClient parent = null;
    int imageNumber = -1;

    public imagePne(pClient c) {

	super();
	parent = c;
	setBackground(highlightColor);
    }

    public void paint(Graphics g) {

	if (imageNumber != -1 && parent.theImages[imageNumber] != null) {
	    g.drawImage(parent.theImages[imageNumber], 0, 0, this);
	}
    }

    public void setImage(int newImage) {

	imageNumber = newImage;
	repaint();
    }
}

